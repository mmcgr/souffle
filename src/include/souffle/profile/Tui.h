/*
 * Souffle - A Datalog Compiler
 * Copyright (c) 2016, The Souffle Developers. All rights reserved
 * Licensed under the Universal Permissive License v 1.0 as shown at:
 * - https://opensource.org/licenses/UPL
 * - <souffle root>/licenses/SOUFFLE-UPL.txt
 */

#pragma once

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <iostream>
#include <memory>
#include <set>
#include <string>
#include <thread>
#include <vector>
#include <dirent.h>
#include <sys/ioctl.h>
#include <sys/stat.h>

namespace souffle {
namespace profile {

/*
 * Text User interface for SouffleProf
 * OutputProcessor creates a ProgramRun object
 * ProgramRun -> Reader.h ProgramRun stores all the data
 * OutputProcessor grabs the data and makes tables
 * Tui displays the data
 */
class Tui {
private:
    bool loaded;
    std::string f_name;
    bool alive = false;
    std::thread updater;
    int sortColumn = 0;
    /// Limit results shown. Default value chosen to approximate unlimited
    size_t resultLimit = 20000;

    struct Usage {
        std::chrono::microseconds time;
        uint64_t maxRSS;
        std::chrono::microseconds systemtime;
        std::chrono::microseconds usertime;
        bool operator<(const Usage& other) const {
            return time < other.time;
        }
    };

public:
    Tui(std::string filename, bool live, bool /* gui */);

    Tui();

    ~Tui();

    void runCommand(std::vector<std::string> c);

    void runProf();

    std::stringstream& genJsonTop(std::stringstream& ss);

    std::stringstream& genJsonRelations(std::stringstream& ss, const std::string& name, size_t maxRows);

    std::stringstream& genJsonRules(std::stringstream& ss, const std::string& name, size_t maxRows);

    std::stringstream& genJsonUsage(std::stringstream& ss);

    std::stringstream& genJsonConfiguration(std::stringstream& ss);

    std::stringstream& genJsonAtoms(std::stringstream& ss);

    std::string genJson();

    void outputHtml(std::string filename = "profiler_html/");

    void quit();

    static void help();

    void usageRelation(std::string id);

    void usageRule(std::string id);

    std::set<Usage> getUsageStats(size_t width = size_t(-1));

    void usage(uint32_t height = 20);

    void usage(std::chrono::microseconds endTime, std::chrono::microseconds startTime, uint32_t height = 20);

    void memoryUsage(uint32_t height = 20);

    void memoryUsage(std::chrono::microseconds /* endTime */, std::chrono::microseconds /* startTime */,
            uint32_t height = 20);
    void setupTabCompletion();

    void configuration();

    void top();

    void setResultLimit(size_t limit) {
        resultLimit = limit;
    }

    void rel(size_t limit, bool showLimit = true);

    void rul(size_t limit, bool showLimit = true);

    void id(std::string col);

    void relRul(std::string str);

    void verRul(std::string str);

    void iterRel(std::string c, std::string col);

    void iterRul(std::string c, std::string col);

    void verGraph(std::string c, std::string col);

    void graphByTime(std::vector<std::chrono::microseconds> list);

    void graphBySize(std::vector<size_t> list);

protected:
    void updateDB();

    uint32_t getTermWidth();
};

}  // namespace profile
}  // namespace souffle
