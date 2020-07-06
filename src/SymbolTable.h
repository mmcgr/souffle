/*
 * Souffle - A Datalog Compiler
 * Copyright (c) 2020, The Souffle Developers. All rights reserved
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
#include <iterator>
#include <mutex>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace souffle {

class SymbolStore {
    static constexpr std::size_t BLOCK_SIZE = 1024 * 1024;

public:
    class const_iterator {
        std::size_t index = 0;
        const SymbolStore& store;

    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = std::string;
        using difference_type = std::size_t;
        using pointer = const std::string*;
        using reference = const std::string&;
        explicit const_iterator(const SymbolStore& store, std::size_t index = 0)
                : index(index), store(store) {}
        const_iterator& operator++() {
            ++index;
            return *this;
        }
        bool operator==(const const_iterator& other) const {
            return index == other.index && &store == &store;
        }
        bool operator!=(const const_iterator& other) const {
            return !operator==(other);
        }
        reference operator*() const {
            return store[index];
        }
        pointer operator->() const {
            return &store[index];
        }
    };

    const_iterator begin() const {
        return const_iterator(*this);
    }
    const_iterator end() const {
        return const_iterator(*this, currentSize);
    }

    std::size_t getNextId() {
        std::size_t result = currentSize++;
        while (symbolBlocks[result / BLOCK_SIZE].load(std::memory_order_relaxed) == nullptr) {
            std::string* oldArray = symbolBlocks[result / BLOCK_SIZE].load(std::memory_order_relaxed);
            if (oldArray != nullptr) {
                break;
            }
            std::string* newArray = new std::string[BLOCK_SIZE];
            if (!symbolBlocks[result / BLOCK_SIZE].compare_exchange_strong(oldArray, newArray)) {
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
        // assert(id < currentSize && "Index out of bounds");
        return symbolBlocks[id / BLOCK_SIZE].load(std::memory_order_relaxed)[id % BLOCK_SIZE];
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

    SymbolStore() = default;

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

private:
    std::unique_ptr<std::atomic<std::string*>[]> symbolBlocks {
        new std::atomic<std::string*>[BLOCK_SIZE] {}
    };
    std::atomic<std::size_t> currentSize{0};

    void insert(std::size_t id, std::string symbol) {
        // assert(id <= currentSize && "Index out of bounds");
        symbolBlocks[id / BLOCK_SIZE].load(std::memory_order_relaxed)[id % BLOCK_SIZE] = symbol;
    }
};

// Concurrent trie map
// Get id by inserting string into SymbolStore
// Add string to id mapping to idstore
//
class IdStore {
    static constexpr std::size_t TRIE_WIDTH = 16;

public:
    struct Node {
        static inline uint8_t getNibble(std::size_t index, const std::string& symbol) {
            uint8_t nibble = symbol[index / 2];
            if (index % 2 == 0) {
                nibble &= 0x0f;
            } else {
                nibble >>= 4;
            }
            return nibble;
        }

        const std::pair<std::size_t, Node*> get(std::size_t index, const std::string& symbol) {
            if (index == 2 * symbol.size()) {
                return {index, this};
            }
            uint8_t nibble = getNibble(index, symbol);
            auto* child = children[nibble].load(std::memory_order_relaxed);
            if (child == nullptr) {
                return {index, this};
            }
            return child->get(index + 1, symbol);
        }

        bool addChild(uint8_t c, Node* node) {
            //	assert(c < TRIE_WIDTH && "out of bounds");
            Node* oldNode = children[c].load(std::memory_order_relaxed);
            if (oldNode != nullptr) {
                return false;
            }
            return children[c].compare_exchange_strong(oldNode, node);
        }

        Node() = default;

        Node(std::size_t id) : id(id) {}

        ~Node() {
            for (auto& child : children) {
                delete child.load();
            }
        }

        Node* clone() const {
            auto* clone = new Node(id);

            for (std::size_t i = 0; i < TRIE_WIDTH; ++i) {
                auto* child = children[i].load();
                if (child == nullptr) {
                    continue;
                }
                clone->children[i].store(child->clone());
            }
            return clone;
        }

        std::atomic<std::size_t> id{0};
        std::array<std::atomic<Node*>, TRIE_WIDTH> children = {};
    };

    std::pair<std::size_t, Node*> get(const std::string& symbol) const {
        return root.load()->get(0, symbol);
    }

    std::size_t getId(const std::string& symbol) const {
        auto resultPair = get(symbol);
        if (resultPair.first == 2 * symbol.size()) {
            return resultPair.second->id;
        }

        throw std::invalid_argument("Symbol not found");
    }

    bool contains(const std::string& symbol) const {
        if (symbol.empty()) {
            return true;
        }
        auto resultPair = get(symbol);
        return resultPair.first == 2 * symbol.size() && resultPair.second->id != 0;
    }

    std::size_t insert(Node* node, std::size_t index, std::size_t id, const std::string& symbol) {
        while (true) {
            // We may be updating the current node with an id, so check this first
            if (index == 2 * symbol.size()) {
                std::size_t oldId = node->id;
                while (oldId == 0 && id != oldId) {
                    node->id.compare_exchange_strong(oldId, id);
                    oldId = node->id;
                }
                return node->id;
            }

            uint8_t nibble = Node::getNibble(index, symbol);
            // Add a new child
            ++index;
            Node* newChild = new Node(index == 2 * symbol.size() ? id : 0);
            if (node->addChild(nibble, newChild)) {
                if (index == 2 * symbol.size()) {
                    return newChild->id;
                }
                return insert(newChild, index, id, symbol);
            }
            // Something went wrong, probably another node already inserted
            // Clean up
            delete newChild;
            --index;

            // Recheck for nearest node
            std::tie(index, node) = node->get(index, symbol);
        }
    }

    std::size_t insert(std::size_t id, const std::string& symbol) {
        if (symbol.empty()) {
            return 0;
        }

        auto nearest = get(symbol);
        if (nearest.first == 2 * symbol.size() && nearest.second->id != 0) {
            return nearest.second->id;
        }
        return insert(nearest.second, nearest.first, id, symbol);
    }

    IdStore() = default;

    IdStore(const IdStore& other) {
        root.store(other.root.load()->clone());
    }

    IdStore(IdStore&& other) {
        root.store(other.root.load());
        other.root.store(nullptr);
    }

    IdStore& operator=(const IdStore& other) {
        delete root.load();
        root.store(other.root.load()->clone());
        return *this;
    }

private:
    std::atomic<Node*> root = new Node(0);
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
    mutable std::mutex access;

    /** Map indices to strings. */
    SymbolStore numToStr;

    /** Map strings to indices. */
    IdStore strToNum;

    /** Convenience method to place a new symbol in the table, if it does not exist, and return the index
     * of it. */
    inline size_t newSymbol(const std::string& symbol) {
        if (symbol.empty()) {
            return 0;
        }
        auto containsPair = strToNum.get(symbol);
        if (containsPair.first == 2 * symbol.size() && containsPair.second->id != 0) {
            return containsPair.second->id;
        }
        // Insert placeholder
        strToNum.insert(containsPair.second, containsPair.first, 0, symbol);

        // Now try updating the placeholder
        containsPair = strToNum.get(symbol);
        std::size_t id = numToStr.insert(symbol);
        return strToNum.insert(containsPair.second, containsPair.first, id, symbol);
    }

public:
    /** Empty constructor. */
    SymbolTable() {
        std::size_t index = numToStr.insert("");
        strToNum.insert(index, "");
    }

    /** Copy constructor, performs a deep copy. */
    SymbolTable(const SymbolTable& other) {
        numToStr = other.numToStr;
        strToNum = other.strToNum;
    }

    SymbolTable(std::initializer_list<std::string> symbols) {
        std::size_t index = numToStr.insert("");
        strToNum.insert(index, "");
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
        return static_cast<RamDomain>(newSymbol(symbol));
    }

    /** Finds the index of a symbol in the table, giving an error if it's not found */
    RamDomain lookupExisting(const std::string& symbol) const {
        return static_cast<RamDomain>(strToNum.getId(symbol));
    }

    /** Find the index of a symbol in the table, inserting a new symbol if it does not exist there
     * already. */
    RamDomain unsafeLookup(const std::string& symbol) {
        return static_cast<RamDomain>(newSymbol(symbol));
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
        for (auto& symbol : symbols) {
            newSymbol(symbol);
        }
    }

    /** Insert a single symbol into the table, not that this operation should not be used if inserting
     * symbols
     * in bulk. */
    void insert(const std::string& symbol) {
        newSymbol(symbol);
    }

    /** Print the symbol table to the given stream. */
    void print(std::ostream& out) const {
        out << "SymbolTable: {\n\t";
        out << join(numToStr);
        out << "}\n";
    }

    /** Check if the symbol table contains a string */
    bool contains(const std::string& symbol) const {
        return strToNum.contains(symbol);
    }

    /** Check if the symbol table contains an index */
    bool contains(const RamDomain index) const {
        return static_cast<std::size_t>(index) < size();
    }

    std::lock_guard<std::mutex> acquireLock() const {
        return std::lock_guard<std::mutex>(access);
    }

    /** Stream operator, used as a convenience for print. */
    friend std::ostream& operator<<(std::ostream& out, const SymbolTable& table) {
        table.print(out);
        return out;
    }
};

}  // namespace souffle
