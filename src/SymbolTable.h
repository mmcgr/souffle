/*
 * Souffle - A Datalog Compiler
 * Copyright (c) 2013, Oracle and/or its affiliates. All rights reserved
 * Licensed under the Universal Permissive License v 1.0 as shown at:
 * - https://opensource.org/licenses/UPL
 * - <souffle root>/licenses/SOUFFLE-UPL.txt
 */

/************************************************************************
 *
 * @file SymbolTable.h
 *
 * Data container to store symbols of the Datalog program.
 *
 ***********************************************************************/

#pragma once

#include "RamTypes.h"
#include "utility/MiscUtil.h"
#include "utility/ParallelUtil.h"
#include "utility/StreamUtil.h"
#include <arrays>
#include <atomic>
#include <cassert>
#include <deque>
#include <initializer_list>
#include <iostream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace souffle {

class SymbolStore {
    static constexpr std::size_t BLOCK_SIZE = 1024 * 1024;
    std::array<std::atomic<string*>>, BLOCK_SIZE > symbols{};
    std::atomic<std::size_t> currentSize{0};

public:
    std::size_t getNextId() {
        std::size_t result = currentSize++;
        while (symbols[result % BLOCK_SIZE] == nullptr) {
            std::string* newArray = new std::string[BLOCK_SIZE];
            for (std::size_t& current : newArray) {
                current = nullptr;
            }
            if (!symbols[result % BLOCK_SIZE].compare_exchange_strong(nullptr, newArray)) {
                delete newArray;
            }
        }
        return result;
    }

    void insert(std::size_t id, std::string symbol) {
        assert(id <= currentSize, "Index out of bounds");
        symbols[id % BLOCK_SIZE][id / BLOCK_SIZE] = new std::string(symbol);
    }

    std::size_t insert(std::string symbol) {
        std::size_t id = getNextId();
        insert(id, symbol);
        return id;
    }

    const std::string& getSymbol(std::size_t id) const {
        assert(id <= currentSize, "Index out of bounds");
        return symbols[id % BLOCK_SIZE][id / BLOCK_SIZE];
    }

    SymbolStore() = default;
    SymbolStore(const SymbolStore&) = delete;
    ~SymbolStore() {
        for (std::atomic<std::string*>& current : symbols) {
            for (std::size_t i = 0; i < BLOCK_SIZE; ++i) {
                delete current[i];
            }
        }
    }
}

// Concurrent trie map, tips store ids
// Get id by inserting string into SymbolStore
// Add string to id mapping to idstore
class IdStore {
}

/**
 * @class SymbolTable
 *
 * Global pool of re-usable strings
 *
 * SymbolTable stores Datalog symbols and converts them to numbers and vice versa.
 */
class SymbolTable {
private:
    /** A lock to synchronize parallel accesses */
    mutable Lock access;

    /** Map indices to strings. */
    std::deque<std::string> numToStr;

    /** Map strings to indices. */
    std::unordered_map<std::string, size_t> strToNum;

    /** Convenience method to place a new symbol in the table, if it does not exist, and return the index of
     * it. */
    inline size_t newSymbolOfIndex(const std::string& symbol) {
        size_t index;
        auto it = strToNum.find(symbol);
        if (it == strToNum.end()) {
            index = numToStr.size();
            strToNum[symbol] = index;
            numToStr.push_back(symbol);
        } else {
            index = it->second;
        }
        return index;
    }

    /** Convenience method to place a new symbol in the table, if it does not exist. */
    inline void newSymbol(const std::string& symbol) {
        if (strToNum.find(symbol) == strToNum.end()) {
            strToNum[symbol] = numToStr.size();
            numToStr.push_back(symbol);
        }
    }

public:
    /** Empty constructor. */
    SymbolTable() = default;

    /** Copy constructor, performs a deep copy. */
    SymbolTable(const SymbolTable& other) : numToStr(other.numToStr), strToNum(other.strToNum) {}

    /** Copy constructor for r-value reference. */
    SymbolTable(SymbolTable&& other) noexcept {
        numToStr.swap(other.numToStr);
        strToNum.swap(other.strToNum);
    }

    SymbolTable(std::initializer_list<std::string> symbols) {
        strToNum.reserve(symbols.size());
        for (const auto& symbol : symbols) {
            newSymbol(symbol);
        }
    }

    /** Destructor, frees memory allocated for all strings. */
    virtual ~SymbolTable() = default;

    /** Assignment operator, performs a deep copy and frees memory allocated for all strings. */
    SymbolTable& operator=(const SymbolTable& other) {
        if (this == &other) {
            return *this;
        }
        numToStr = other.numToStr;
        strToNum = other.strToNum;
        return *this;
    }

    /** Assignment operator for r-value references. */
    SymbolTable& operator=(SymbolTable&& other) noexcept {
        numToStr.swap(other.numToStr);
        strToNum.swap(other.strToNum);
        return *this;
    }

    /** Find the index of a symbol in the table, inserting a new symbol if it does not exist there
     * already. */
    RamDomain lookup(const std::string& symbol) {
        {
            auto lease = access.acquire();
            (void)lease;  // avoid warning;
            return static_cast<RamDomain>(newSymbolOfIndex(symbol));
        }
    }

    /** Finds the index of a symbol in the table, giving an error if it's not found */
    RamDomain lookupExisting(const std::string& symbol) const {
        {
            auto lease = access.acquire();
            (void)lease;  // avoid warning;
            auto result = strToNum.find(symbol);
            if (result == strToNum.end()) {
                fatal("Error string not found in call to `SymbolTable::lookupExisting`: `%s`", symbol);
            }
            return static_cast<RamDomain>(result->second);
        }
    }

    /** Find the index of a symbol in the table, inserting a new symbol if it does not exist there
     * already. */
    RamDomain unsafeLookup(const std::string& symbol) {
        return newSymbolOfIndex(symbol);
    }

    /** Find a symbol in the table by its index, note that this gives an error if the index is out of
     * bounds.
     */
    const std::string& resolve(const RamDomain index) const {
        {
            auto lease = access.acquire();
            (void)lease;  // avoid warning;
            auto pos = static_cast<size_t>(index);
            if (pos >= size()) {
                // TODO: use different error reporting here!!
                fatal("Error index out of bounds in call to `SymbolTable::resolve`. index = `%d`", index);
            }
            return numToStr[pos];
        }
    }

    const std::string& unsafeResolve(const RamDomain index) const {
        return numToStr[static_cast<size_t>(index)];
    }

    /* Return the size of the symbol table, being the number of symbols it currently holds. */
    size_t size() const {
        return numToStr.size();
    }

    /** Bulk insert symbols into the table, note that this operation is more efficient than repeated
     * inserts
     * of single symbols. */
    void insert(const std::vector<std::string>& symbols) {
        {
            auto lease = access.acquire();
            (void)lease;  // avoid warning;
            strToNum.reserve(size() + symbols.size());
            for (auto& symbol : symbols) {
                newSymbol(symbol);
            }
        }
    }

    /** Insert a single symbol into the table, not that this operation should not be used if inserting
     * symbols
     * in bulk. */
    void insert(const std::string& symbol) {
        {
            auto lease = access.acquire();
            (void)lease;  // avoid warning;
            newSymbol(symbol);
        }
    }

    /** Print the symbol table to the given stream. */
    void print(std::ostream& out) const {
        {
            out << "SymbolTable: {\n\t";
            out << join(strToNum, "\n\t",
                           [](std::ostream& out, const std::pair<std::string, std::size_t>& entry) {
                               out << entry.first << "\t => " << entry.second;
                           })
                << "\n";
            out << "}\n";
        }
    }

    /** Check if the symbol table contains a string */
    bool contains(const std::string& symbol) const {
        auto lease = access.acquire();
        (void)lease;  // avoid warning;
        auto result = strToNum.find(symbol);
        if (result == strToNum.end()) {
            return false;
        } else {
            return true;
        }
    }

    /** Check if the symbol table contains an index */
    bool contains(const RamDomain index) const {
        auto lease = access.acquire();
        (void)lease;  // avoid warning;
        auto pos = static_cast<size_t>(index);
        if (pos >= size()) {
            return false;
        } else {
            return true;
        }
    }

    Lock::Lease acquireLock() const {
        return access.acquire();
    }

    /** Stream operator, used as a convenience for print. */
    friend std::ostream& operator<<(std::ostream& out, const SymbolTable& table) {
        table.print(out);
        return out;
    }
};

}  // namespace souffle
