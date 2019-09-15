/*
 * Souffle - A Datalog Compiler
 * Copyright (c) 2016, The Souffle Developers. All rights reserved
 * Licensed under the Universal Permissive License v 1.0 as shown at:
 * - https://opensource.org/licenses/UPL
 * - <souffle root>/licenses/SOUFFLE-UPL.txt
 */

#pragma once

#include "CellInterface.h"

#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace souffle {
namespace profile {

/*
 * Row class for Tables, holds a vector of cells.
 */
class Row {
public:
    std::vector<std::shared_ptr<CellInterface>> cells;

    Row(unsigned long size) : cells() {
        for (unsigned long i = 0; i < size; i++) {
            cells.emplace_back(std::shared_ptr<CellInterface>(nullptr));
        }
    }

    inline std::vector<std::string> toStringVector(int precision) const {
        std::vector<std::string> strings;
        for (auto& cell : cells) {
            strings.push_back(cell->toString(precision));
        }
        return strings;
    }

    inline void setCell(size_t i, std::shared_ptr<CellInterface> cell) {
        cells.at(i) = cell;
    }

    inline std::string valueToString(size_t i, int precision) {
        if (cells.at(i) == nullptr) {
            return "-";
        }
        return cells.at(i)->toString(precision);
    }

    inline double getDoubleValue(size_t i) {
        return cells.at(i)->getDoubleVal();
    }

    inline long getLongValue(size_t i) {
        return cells.at(i)->getLongVal();
    }

    inline std::string getStringValue(size_t i) {
        return cells.at(i)->getStringVal();
    }

    inline std::chrono::microseconds getTimeValue(size_t i) {
        return cells.at(i)->getTimeVal();
    }
};

}  // namespace profile
}  // namespace souffle
