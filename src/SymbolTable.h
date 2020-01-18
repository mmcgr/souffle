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
#include <array>
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
    std::unique_ptr<std::atomic<std::string*>[]> symbolBlocks { new std::atomic<std::string*>[ BLOCK_SIZE ] };
    std::atomic<std::size_t> currentSize{0};

    void insert(std::size_t id, std::string symbol) {
        assert(id <= currentSize && "Index out of bounds");
        symbolBlocks[id % BLOCK_SIZE][id / BLOCK_SIZE] = symbol;
    }

public:
    std::size_t getNextId() {
        std::size_t result = currentSize++;
        while (symbolBlocks[result % BLOCK_SIZE] == nullptr) {
            std::string* oldArray = symbolBlocks[result % BLOCK_SIZE].load();
            if (oldArray != nullptr) {
                break;
            }
            std::string* newArray = new std::string[BLOCK_SIZE];
            if (!symbolBlocks[result % BLOCK_SIZE].compare_exchange_strong(oldArray, newArray)) {
                delete[] newArray;
            }
        }
        return result;
    }

    std::size_t insert(std::string symbol) {
        std::size_t id = getNextId();
        insert(id, symbol);
        return id;
    }

    const std::string& operator[](std::size_t id) const {
        assert(id < currentSize && "Index out of bounds");
        return symbolBlocks[id % BLOCK_SIZE][id / BLOCK_SIZE];
    }

    std::size_t size() const {
        return currentSize;
    }

    void clear() {
        currentSize = 0;
        for (std::size_t i = 0; i < BLOCK_SIZE; ++i) {
            delete[] symbolBlocks[i].load();
            symbolBlocks[i].store(nullptr);
        }
    }

    SymbolStore() {
        for (std::size_t i = 0; i < BLOCK_SIZE; ++i) {
            symbolBlocks[i].store(nullptr);
        }
    }

    SymbolStore(const SymbolStore& other) {
        for (std::size_t i = 0; i < BLOCK_SIZE; ++i) {
            symbolBlocks[i].store(nullptr);
        }
        for (std::size_t i = 0; i < other.currentSize; ++i) {
            insert(other[i]);
        }
    }

    SymbolStore& operator=(const SymbolStore& other) {
        clear();
        for (std::size_t i = 0; i < other.currentSize; ++i) {
            insert(other[i]);
        }
        return *this;
    }

    ~SymbolStore() {
        clear();
    }
};

// Concurrent trie map, tips store ids
// Get id by inserting string into SymbolStore
// Add string to id mapping to idstore
//
// Hmm. TODO: mark deleted nodes? Currently we risk reusing a deleted parent
class IdStore {
public:
    struct IdStoreNode {
        std::atomic<std::size_t> id{0};
        std::atomic<std::unordered_map<char, std::atomic<IdStoreNode*>>*> children =
                new std::unordered_map<char, std::atomic<IdStoreNode*>>;
        IdStoreNode* get(char c) {
            auto it = children.load()->find(c);
            if (it == children.load()->end()) {
                return nullptr;
            }
            return it->second;
        }
        IdStoreNode() = default;
        IdStoreNode(std::size_t id, std::unordered_map<char, std::atomic<IdStoreNode*>>* children)
                : id(id), children(children) {}
        bool addChild(char c, IdStoreNode* node) {
            auto* oldChildren = children.load();
            if (oldChildren->count(c) != 0) {
                return false;
            }
            auto* newChildren = new std::unordered_map<char, std::atomic<IdStoreNode*>>();
            for (auto& pair : *oldChildren) {
                (*newChildren)[pair.first].store(pair.second);
            }
            (*newChildren)[c].store(node);
            if (children.compare_exchange_strong(oldChildren, newChildren)) {
                delete oldChildren;
                return true;
            } else {
                delete newChildren;
                return false;
            }
        }
        ~IdStoreNode() {
            for (auto& child : *children) {
                delete child.second;
            }
            delete children.load();
        }
    };
    std::atomic<IdStoreNode*> root = new IdStoreNode(0, {});

    IdStoreNode* get(const std::string& symbol) {
        IdStoreNode* current = root;
        for (char c : symbol) {
            current = current->get(c);
            if (current == nullptr) {
                break;
            }
        }
        return current;
    }
    std::size_t getId(const std::string& symbol) {
        IdStoreNode* node = get(symbol);
        if (node == nullptr) {
            throw std::invalid_argument("Symbol not found");
        }
        return node->id;
    }

    bool contains(const std::string& symbol) {
        return get(symbol) != nullptr;
    }
    std::size_t insert(std::size_t id, const std::string& symbol) {
        if (symbol.empty()) {
            return 0;
        }
        // Check root node
        while (root.load()->get(symbol.front()) == nullptr) {
            IdStoreNode* newChild = makeChain(id, symbol, 0);
            if (root.load()->addChild(symbol.front(), newChild)) {
                return id;
            }
            delete newChild;
        }
        // Search for matching node
        IdStoreNode* parent = root;
        IdStoreNode* current = parent->get(symbol.front());
        IdStoreNode* next = nullptr;
        std::size_t i = 1;
        for (; i < symbol.size();) {
            char c = symbol[i];
            next = current->get(c);
            if (next != nullptr) {
                parent = current;
                current = next;
                ++i;
                continue;
            }
            IdStoreNode* newChild = makeChain(id, symbol, i);
            if (current->addChild(symbol[i], newChild)) {
                return id;
            } else {
                delete newChild;
            }
        }
        // A matching node is found - either it's a leaf and we return the existing id
        // Or it's not and we insert the id
        if ((*current->children).empty()) {
            return current->id;
        }
        auto cId = current->id.load();
        // Found a matching node. Now we either retur the id or set it
        if (cId > 0) {
            return cId;
        }
        // If the id is 0 update it
        current->id.compare_exchange_strong(cId, id);
        return current->id;
    }
    IdStoreNode* makeChain(std::size_t id, std::string symbol, std::size_t index) {
        IdStoreNode* root = new IdStoreNode();
        if (index < symbol.size()) {
            (*root->children)[symbol[index]].store(makeChain(id, symbol, index + 1));
        } else {
            root->id = id;
        }
        return root;
    }
};

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
    SymbolStore numToStr;

    /** Map strings to indices. */
    std::unordered_map<std::string, size_t> strToNum;

    /** Convenience method to place a new symbol in the table, if it does not exist, and return the index
     * of it. */
    inline size_t newSymbolOfIndex(const std::string& symbol) {
        size_t index;
        auto it = strToNum.find(symbol);
        if (it == strToNum.end()) {
            index = numToStr.insert(symbol);
            strToNum[symbol] = index;
        } else {
            index = it->second;
        }
        return index;
    }

    /** Convenience method to place a new symbol in the table, if it does not exist. */
    inline void newSymbol(const std::string& symbol) {
        if (strToNum.find(symbol) == strToNum.end()) {
            std::size_t index = numToStr.insert(symbol);
            strToNum[symbol] = index;
        }
    }

public:
    /** Empty constructor. */
    SymbolTable() = default;

    /** Copy constructor, performs a deep copy. */
    SymbolTable(const SymbolTable& other) {
        std::size_t size = other.size();
        for (std::size_t i = 0; i < size; ++i) {
            newSymbol(other.resolve(i));
        }
    }

    SymbolTable(std::initializer_list<std::string> symbols) {
        for (const auto& symbol : symbols) {
            newSymbol(symbol);
        }
    }

    SymbolTable& operator=(const SymbolTable& other) {
        numToStr = other.numToStr;
        strToNum = other.strToNum;
        return *this;
    }

    /** Destructor, frees memory allocated for all strings. */
    virtual ~SymbolTable() = default;

    /** Find the index of a symbol in the table, inserting a new symbol if it does not exist there
     * already. */
    RamDomain lookup(const std::string& symbol) {
        auto lease = access.acquire();
        (void)lease;  // avoid warning;
        return static_cast<RamDomain>(newSymbolOfIndex(symbol));
    }

    /** Finds the index of a symbol in the table, giving an error if it's not found */
    RamDomain lookupExisting(const std::string& symbol) const {
        auto lease = access.acquire();
        (void)lease;  // avoid warning;
        auto result = strToNum.find(symbol);
        if (result == strToNum.end()) {
            fatal("Error string not found in call to `SymbolTable::lookupExisting`: `%s`", symbol);
        }
        return static_cast<RamDomain>(result->second);
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
        return numToStr[static_cast<size_t>(index)];
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
        auto lease = access.acquire();
        (void)lease;  // avoid warning;
        for (auto& symbol : symbols) {
            newSymbol(symbol);
        }
    }

    /** Insert a single symbol into the table, not that this operation should not be used if inserting
     * symbols
     * in bulk. */
    void insert(const std::string& symbol) {
        auto lease = access.acquire();
        (void)lease;  // avoid warning;
        newSymbol(symbol);
    }

    /** Print the symbol table to the given stream. */
    void print(std::ostream& out) const {
        out << "SymbolTable: {\n\t";
        out << join(strToNum, "\n\t",
                       [](std::ostream& out, const std::pair<std::string, std::size_t>& entry) {
                           out << entry.first << "\t => " << entry.second;
                       })
            << "\n";
        out << "}\n";
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
        return static_cast<std::size_t>(index) < size();
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
