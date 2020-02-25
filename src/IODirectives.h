/*
 * Souffle - A Datalog Compiler
 * Copyright (c) 2013, 2014, Oracle and/or its affiliates. All rights reserved
 * Licensed under the Universal Permissive License v 1.0 as shown at:
 * - https://opensource.org/licenses/UPL
 * - <souffle root>/licenses/SOUFFLE-UPL.txt
 */

/************************************************************************
 *
 * @file IODirectives.h
 *
 ***********************************************************************/

#pragma once

#include <map>
#include <regex>
#include <sstream>
#include <string>

namespace souffle {

class IODirectives {
public:
    IODirectives() = default;

    IODirectives(const std::map<std::string, std::string>& directiveMap) {
        for (const auto& pair : directiveMap) {
            directives[pair.first] = pair.second;
        }
    }

    ~IODirectives() = default;

    const std::string& getIOType() const {
        return get("IO");
    }

    void setIOType(const std::string& type) {
        directives["IO"] = type;
    }

    const std::string& get(const std::string& key) const {
        if (directives.count(key) == 0) {
            throw std::invalid_argument("Requested IO directive <" + key + "> was not specified");
        }
        return directives.at(key);
    }

    const std::map<std::string, std::string>& getMap() const {
        return directives;
    }

    void set(const std::string& key, const std::string& value) {
        directives[key] = value;
    }

    /** Basic conversion of escaped input (\", \t, \r, \n) */
    void cleanAndSet(const std::string& key, const std::string& value) {
        directives[key] = unescape(value);
    }

    bool has(const std::string& key) const {
        return directives.count(key) > 0;
    }

    const std::string& getFileName() const {
        return get("filename");
    }

    void setFileName(const std::string& filename) {
        directives["filename"] = filename;
    }

    const std::string& getRelationName() const {
        return get("name");
    }

    void setRelationName(const std::string& name) {
        directives["name"] = name;
    }

    bool isEmpty() {
        return directives.empty();
    }

    void print(std::ostream& out) const {
        auto cur = directives.begin();
        if (cur == directives.end()) {
            return;
        }

        out << "{{\"" << cur->first << "\",\"" << escape(cur->second) << "\"}";
        ++cur;
        for (; cur != directives.end(); ++cur) {
            out << ",{\"" << cur->first << "\",\"" << escape(cur->second) << "\"}";
        }
        out << '}';
    }

    friend std::ostream& operator<<(std::ostream& out, const IODirectives& ioDirectives) {
        ioDirectives.print(out);
        return out;
    }

    bool operator==(const IODirectives& other) const {
        return directives == other.directives;
    }

    bool operator!=(const IODirectives& other) const {
        return directives != other.directives;
    }

private:
    std::string escape(std::string text) const {
        text = std::regex_replace(text, std::regex("\""), "\\\"");
        text = std::regex_replace(text, std::regex("\t"), "\\t");
        text = std::regex_replace(text, std::regex("\r"), "\\r");
        text = std::regex_replace(text, std::regex("\n"), "\\n");
        return text;
    }

    std::string unescape(std::string text) {
        text = std::regex_replace(text, std::regex("\\\""), "\"");
        text = std::regex_replace(text, std::regex("\\t"), "\t");
        text = std::regex_replace(text, std::regex("\\r"), "\r");
        text = std::regex_replace(text, std::regex("\\n"), "\n");
        return text;
    }

    std::map<std::string, std::string> directives;
};
}  // namespace souffle
