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
#include <thread>
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

    void returnId(std::size_t unusedId) {
        std::size_t newId = unusedId - 1;
        currentSize.compare_exchange_strong(unusedId, newId);
        --usedSize;
    }

    std::size_t insert(std::string symbol) {
        std::size_t id = getNextId();
        ++usedSize;
        insert(id, symbol);
        return id;
    }

    const std::string& operator[](std::size_t id) const {
        // assert(id < currentSize && "Index out of bounds");
        return symbolBlocks[id / BLOCK_SIZE].load(std::memory_order_relaxed)[id % BLOCK_SIZE];
    }

    std::size_t size() const {
        return usedSize;
    }

    void clear() {
        currentSize = 0;
        for (std::size_t i = 0; i < BLOCK_SIZE; ++i) {
            delete[] symbolBlocks[i].load();
            symbolBlocks[i].store(nullptr);
        }
    }

    void print(std::ostream& os) const {
        if (currentSize > 0) {
            os << "1:" << symbolBlocks[1 / BLOCK_SIZE].load(std::memory_order_relaxed)[1 % BLOCK_SIZE];
        }
        for (std::size_t id = 2; id < currentSize; ++id) {
            os << ',' << id << ':'
               << symbolBlocks[id / BLOCK_SIZE].load(std::memory_order_relaxed)[id % BLOCK_SIZE];
        }
    }

    SymbolStore() {
        getNextId();
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

private:
    std::unique_ptr<std::atomic<std::string*>[]> symbolBlocks {
        new std::atomic<std::string*>[BLOCK_SIZE] {}
    };
    std::atomic<std::size_t> currentSize{0};
    std::atomic<std::size_t> usedSize{0};

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

        /**
         * Returns the nearest node to the end of a given symbol
         * Return value is a pair of index in the symbol and the found node.
         */
        const std::pair<std::size_t, Node*> get(std::size_t index, const std::string& symbol) {
            // Stop searching if we've reached the length of the symbol
            if (index == 2 * symbol.size()) {
                return {index, this};
            }
            uint8_t nibble = getNibble(index, symbol);
            auto* child = children[nibble].load(std::memory_order_seq_cst);
            // No matching child so return this intermediate node
            if (child == nullptr) {
                return {index, this};
            }
            // Try the next child
            return child->get(index + 1, symbol);
        }

        bool addChild(uint8_t c, Node* node) {
            //	assert(c < TRIE_WIDTH && "out of bounds");
            Node* oldNode = children[c].load(std::memory_order_seq_cst);
            if (oldNode != nullptr) {
                return false;
            }
            std::size_t newId = node->id;
            if (children[c].compare_exchange_strong(oldNode, node)) {
                // If the new child has the same id, then reset this one to 0
                while (newId != 0 && newId == id) {
                    // this id matches the new child id, and nothing else so we're safe to modify length
                    length.store(0);
                    id.compare_exchange_strong(newId, 0);
                    newId = node->id;
                }
                return true;
            }
            return false;
        }

        void print(std::ostream& os, std::size_t indentCount = 0) const {
            // for (std::size_t i = 0; i < indentCount; ++i) { std::cout << ' '; }
            os << "{\n";
            for (std::size_t i = 0; i < indentCount + 1; ++i) {
                std::cout << ' ';
            }
            os << "id: " << id << '\n';
            for (std::size_t i = 0; i < indentCount + 1; ++i) {
                std::cout << ' ';
            }
            bool foundChildren = false;
            os << "children: {";
            for (std::size_t i = 0; i < TRIE_WIDTH; ++i) {
                if (children[i] != nullptr) {
                    foundChildren = true;
                    os << "\n";
                    for (std::size_t i = 0; i < indentCount + 2; ++i) {
                        std::cout << ' ';
                    }
                    os << i << ": ";
                    children[i].load()->print(os, indentCount + 2);
                }
            }
            if (foundChildren) {
                for (std::size_t i = 0; i < indentCount + 1; ++i) {
                    std::cout << ' ';
                }
            }
            os << "}\n";
            for (std::size_t i = 0; i < indentCount; ++i) {
                std::cout << ' ';
            }
            os << "}\n";
        }

        Node(std::size_t id, std::uint32_t length) : id(id), length(length) {}

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
        std::atomic<std::uint32_t> length{0};
        std::array<std::atomic<Node*>, TRIE_WIDTH> children = {};
    };

    std::pair<std::size_t, Node*> get(const std::string& symbol) const {
        return root->get(0, symbol);
    }

    std::size_t getId(const std::string& symbol) const {
        auto resultPair = get(symbol);
        return resultPair.second->id;
    }

    std::size_t contains(const std::string& symbol) const {
        if (symbol.empty()) {
            return root->id;
        }
        auto resultPair = get(symbol);
        return resultPair.second->id;
    }

    std::size_t insert(
            Node* node, std::size_t index, std::size_t id, const std::string& symbol, bool debug = false) {
        while (true) {
            // We may be updating the current node with an id, so check this first
            if (index == 2 * symbol.size()) {
                std::size_t oldId = node->id;
                while (oldId == 0 && oldId != id) {
                    node->id.compare_exchange_strong(oldId, id);
                    oldId = node->id;
                }
                return node->id;
            }

            uint8_t nibble = Node::getNibble(index, symbol);
            std::size_t existingId = node->id;
            // node has an id of 0 so it cannot be a stub - insert away
            // (even if the id changes, it's still not a stub
            if (existingId == 0) {
                Node* newChild = new Node();
                node->addChild(nibble, newChild);

                // Either we added a new child node or someone else did
                std::tie(index, node) = node->get(index, symbol);
                continue;
            }
            // Add a new child
            Node* newChild = new Node(id, symbol.size());
            if (node->addChild(nibble, newChild)) {
                return id;
            }
            // Something went wrong, probably another node already inserted
            // Clean up
            delete newChild;

            // Recheck for nearest node
            std::tie(index, node) = node->get(index, symbol);
            continue;
        }
    }

    std::size_t insert(std::size_t id, const std::string& symbol) {
        auto nearest = get(symbol);
        // Symbol exists and we've found it
        if (nearest.first == 2 * symbol.size() && nearest.second->id != 0) {
            return nearest.second->id;
        }
        return insert(nearest.second, nearest.first, id, symbol);
    }

    void print(std::ostream& os) const {
        root->print(os);
    }

    IdStore() = default;

    IdStore(IdStore&& other) {
        std::swap(root, other.root);
    }

    IdStore& operator=(const IdStore& other) {
        delete root;
        root = other.root->clone();
        return *this;
    }

private:
    Node* root = new Node(0);
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
    // TODO (mmcgr): Remove use of this mutex via acquireLock()
    /** A lock to synchronize parallel accesses */
    mutable std::mutex access;

    /** Map indices to strings. */
    SymbolStore numToStr;

    /** Map strings to indices. */
    IdStore strToNum;

    /** Number of successfully inserted symbols */
    std::atomic<std::size_t> currentSize{0};

    /** Convenience method to place a new symbol in the table, if it does not exist, and return the index
     * of it. */
    inline size_t newSymbol(const std::string& symbol) {
        auto containsPair = strToNum.get(symbol);

        std::size_t foundId = containsPair.second->id.load(std::memory_order_seq_cst);
        // Nearest node represents a symbol
        if (foundId != 0 && numToStr[foundId].size() * 2 == containsPair.first &&
                symbol.size() * 2 == containsPair.first) {
            return foundId;
        }
        std::size_t loopCount = 0;
        // Check if the node is a stub and if so, extend it and try again.
        while (foundId != 0 && containsPair.first != 2 * numToStr[foundId].size()) {
            ++loopCount;
            if (loopCount > 1000) {
                auto guard = acquireLock();
                std::this_thread::sleep_for(std::chrono::milliseconds(10000));
                std::cout << "attempting to insert " << symbol << std::endl;
                std::cout << "found id = " << foundId << std::endl;
                std::cout << "index = " << containsPair.first << std::endl;
                std::cout << "id resolves to " << numToStr[foundId] << std::endl;
                std::cout << "of length " << numToStr[foundId].size() << std::endl;
                containsPair.second->print(std::cout);
                auto newId = strToNum.insert(
                        containsPair.second, containsPair.first, foundId, numToStr[foundId], true);
                std::cout << "id of new insertion attempt = " << newId << std::endl;
                std::cout << std::endl;
                containsPair.second->print(std::cout);
                // strToNum.print(std::cout);
                std::cout << std::endl;
                exit(0);
            }
            assert(loopCount < 1001);
            strToNum.insert(containsPair.second, containsPair.first, foundId, numToStr[foundId]);
            containsPair = strToNum.get(symbol);
            foundId = containsPair.second->id.load(std::memory_order_seq_cst);
        }

        // Nearest node is an intermediate to our target
        // Insert placeholder
        strToNum.insert(containsPair.second, containsPair.first, 0, symbol);
        containsPair = strToNum.get(symbol);

        // get a new id and insert it
        std::size_t id = numToStr.insert(symbol);
        std::size_t resultId = strToNum.insert(containsPair.second, containsPair.first, id, symbol);
        // If the symbol already existed, attempt to put back the new id.
        if (id != resultId) {
            numToStr.returnId(id);
        } else {
            ++currentSize;
        }
        return resultId;
    }

public:
    /** Empty constructor. */
    SymbolTable() = default;

    /** Copy constructor, performs a deep copy. */
    SymbolTable(const SymbolTable& other) {
        numToStr = other.numToStr;
        strToNum = other.strToNum;
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
        return static_cast<RamDomain>(newSymbol(symbol));
    }

    /** Finds the index of a symbol in the table, giving an error if it's not found */
    RamDomain lookupExisting(const std::string& symbol) const {
        std::size_t id = strToNum.getId(symbol);
        if (numToStr[id] == symbol) {
            return static_cast<RamDomain>(id);
        }
        throw std::invalid_argument("Symbol not found");
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
        return currentSize;
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
        numToStr.print(out);
        //        if (numToStr.size() > 1) {
        //            out << numToStr[1];
        //        }
        //        for (std::size_t i = 2; i < numToStr.size(); ++i) {
        //            out << ',' << numToStr[i];
        //        }
        out << "\n}\n";
    }

    /** Check if the symbol table contains a string */
    bool contains(const std::string& symbol) const {
        std::size_t id = strToNum.contains(symbol);
        if (id == 0) {
            return false;
        }
        return numToStr[id] == symbol;
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
