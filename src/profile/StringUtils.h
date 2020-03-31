/*
 * Souffle - A Datalog Compiler
 * Copyright (c) 2016, The Souffle Developers. All rights reserved
 * Licensed under the Universal Permissive License v 1.0 as shown at:
 * - https://opensource.org/licenses/UPL
 * - <souffle root>/licenses/SOUFFLE-UPL.txt
 */

#pragma once

#include "CellInterface.h"
#include "Row.h"
#include "Table.h"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <iomanip>
#include <ios>
#include <memory>
#include <sstream>
#include <string>
#include <vector>
#include <unistd.h>

#include <sys/stat.h>

namespace souffle {
namespace profile {

/*
 * A series of functions necessary throughout the code
 * Mostly string manipulation
 */
namespace Tools {
static const std::vector<std::string> abbreviations{
        "K", "M", "B", "t", "q", "Q", "s", "S", "o", "n", "d", "U"};

inline std::string formatNum(double amount) {
    std::stringstream ss;
    ss << amount;
    return ss.str();
}

inline std::string formatNum(int precision, int64_t amount) {
    // assumes number is < 999*10^12
    if (amount == 0) {
        return "0";
    }

    if (precision <= 0) {
        return std::to_string(amount);
    }

    std::string result;

    if (amount < 1000) {
        return std::to_string(amount);
    }

    for (size_t i = 0; i < abbreviations.size(); ++i) {
        if (amount > std::pow(1000, i + 2)) {
            continue;
        }

        double r = amount / std::pow(1000, i + 1);
        result = std::to_string(r);

        if (r >= 100) {  // 1000 > result >= 100

            switch (precision) {
                case 1:
                    result = result.substr(0, 1) + "00";
                    break;
                case 2:
                    result = result.substr(0, 2) + "0";
                    break;
                case 3:
                    result = result.substr(0, 3);
                    break;
                default:
                    result = result.substr(0, precision + 1);
            }
        } else if (r >= 10) {  // 100 > result >= 10
            switch (precision) {
                case 1:
                    result = result.substr(0, 1) + "0";
                    break;
                case 2:
                    result = result.substr(0, 2);
                    break;
                default:
                    result = result.substr(0, precision + 1);
            }
        } else {  // 10 > result > 0
            switch (precision) {
                case 1:
                    result = result.substr(0, 1);
                    break;
                default:
                    result = result.substr(0, precision + 1);
            }
        }
        result += abbreviations.at(i);
        return result;
    }
    // If we ever have integers too large to handle, fall back to this
    return std::to_string(amount);
}

inline std::string formatMemory(uint64_t kbytes) {
    if (kbytes < 1024L * 2) {
        return std::to_string(kbytes) + "kB";
    } else if (kbytes < 1024L * 1024 * 2) {
        return std::to_string(kbytes / 1024) + "MB";
    } else if (kbytes < 1024L * 1024 * 1024 * 2) {
        return std::to_string(kbytes / (1024 * 1024)) + "GB";
    }
    return std::to_string(kbytes / (1024 * 1024 * 1024)) + "TB";
}

inline std::string formatTime(std::chrono::microseconds number) {
    uint64_t sec = number.count() / 1000000;
    if (sec >= 100) {
        uint64_t min = std::floor(sec / 60);
        if (min >= 100) {
            uint64_t hours = std::floor(min / 60);
            if (hours >= 100) {
                uint64_t days = std::floor(hours / 24);
                return std::to_string(days) + "D";
            }
            return std::to_string(hours) + "h";
        }
        if (min < 10) {
            // temp should always be 1 digit long
            uint64_t temp = std::floor((sec - (min * 60.0)) * 10.0 / 6.0);
            return std::to_string(min) + "." + std::to_string(temp).substr(0, 1) + "m";
        }
        return std::to_string(min) + "m";
    } else if (sec >= 10) {
        return std::to_string(sec) + "s";
    } else if (number.count() >= 1000000) {
        std::string temp = std::to_string(number.count() / 100);
        return temp.substr(0, 1) + "." + temp.substr(1, 2) + "s";
    } else if (number.count() >= 100000) {
        std::string temp = std::to_string(number.count() / 1000);
        return "." + temp.substr(0, 3) + "s";
    } else if (number.count() >= 10000) {
        std::string temp = std::to_string(number.count() / 1000);
        return ".0" + temp.substr(0, 2) + "s";
    } else if (number.count() >= 1000) {
        std::string temp = std::to_string(number.count() / 1000);
        return ".00" + temp.substr(0, 1) + "s";
    }

    return ".000s";
}

inline std::vector<std::vector<std::string>> formatTable(Table table, int precision) {
    std::vector<std::vector<std::string>> result;
    for (auto& row : table.getRows()) {
        result.push_back(row->toStringVector(precision));
    }
    return result;
}

/** @brief split on the delimiter */
inline std::vector<std::string> split(std::string toSplit, std::string delimiter) {
    std::vector<std::string> elements;
    std::string::size_type lastPos = 0;
    auto pos = toSplit.find(delimiter, lastPos);

    while (pos != std::string::npos) {
        if (pos > 0) {
            std::string newElement = toSplit.substr(lastPos, pos - lastPos);
            elements.push_back(newElement);
        }
        lastPos = pos + delimiter.size();
        pos = toSplit.find(delimiter, lastPos);
    }
    if (lastPos < toSplit.size()) {
        elements.push_back(toSplit.substr(lastPos));
    }

    return elements;
}

inline std::string trimWhitespace(std::string str) {
    std::string whitespace = " \t";
    size_t first = str.find_first_not_of(whitespace);
    if (first != std::string::npos) {
        str.erase(0, first);
        size_t last = str.find_last_not_of(whitespace);
        str.erase(last + 1);
    } else {
        str.clear();
    }

    return str;
}

/** @brief Remove \n and \t characters, \n and \t sequence of two chars, and wrapping quotes */
inline std::string cleanString(std::string val, std::string replacement = " ") {
    if (val.size() < 2) {
        return val;
    }

    if (val.at(0) == '"' && val.at(val.size() - 1) == '"') {
        val = val.substr(1, val.size() - 2);
    }

    size_t start_pos = 0;
    while ((start_pos = val.find('\\', start_pos)) != std::string::npos) {
        val.erase(start_pos, 1);
        if (start_pos < val.size()) {
            if (val[start_pos] == 'n' || val[start_pos] == 't') {
                val.replace(start_pos, 1, "\t");
            }
        }
    }

    std::replace(val.begin(), val.end(), '\n', '\t');

    // Replace whitespace strings formerly containing \n or \t with a single space
    start_pos = 0;
    while ((start_pos = val.find('\t', start_pos)) != std::string::npos) {
        size_t firstWhitespace = start_pos;
        size_t lastWhitespace = start_pos;

        while (true) {
            if (val[firstWhitespace] != '\t' && val[firstWhitespace] != ' ') {
                ++firstWhitespace;
                break;
            }
            if (firstWhitespace == 0) {
                break;
            }
            --firstWhitespace;
        }
        while (lastWhitespace < val.size()) {
            if (val[lastWhitespace] != '\t' && val[lastWhitespace] != ' ') {
                --lastWhitespace;
                break;
            }
            ++lastWhitespace;
        }

        val.replace(firstWhitespace, 1 + lastWhitespace - firstWhitespace, replacement);
    }

    return val;
}

/** @brief escape escapes and quotes, and remove surrounding quotes */
inline std::string cleanJsonOut(std::string value) {
    if (value.size() >= 2) {
        if (value.at(0) == '"' && value.at(value.size() - 1) == '"') {
            value = value.substr(1, value.size() - 2);
        }
    }

    size_t start_pos = 0;
    while ((start_pos = value.find('\\', start_pos)) != std::string::npos) {
        value.replace(start_pos, 1, "\\\\");
        start_pos += 2;
    }
    start_pos = 0;
    while ((start_pos = value.find('"', start_pos)) != std::string::npos) {
        value.replace(start_pos, 1, "\\\"");
        start_pos += 2;
    }
    return value;
}

/** @brief Convert doubles to NaN or scientific notation */
inline std::string cleanJsonOut(double val) {
    if (std::isnan(val)) {
        return "NaN";
    }
    std::ostringstream ss;
    ss << std::scientific << std::setprecision(6) << val;
    return ss.str();
}
}  // namespace Tools

}  // namespace profile
}  // namespace souffle
