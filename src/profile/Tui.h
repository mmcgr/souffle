/*
 * Souffle - A Datalog Compiler
 * Copyright (c) 2016, The Souffle Developers. All rights reserved
 * Licensed under the Universal Permissive License v 1.0 as shown at:
 * - https://opensource.org/licenses/UPL
 * - <souffle root>/licenses/SOUFFLE-UPL.txt
 */

#pragma once

#include "../ProfileEvent.h"
#include "HtmlGenerator.h"
#include "OutputProcessor.h"
#include "Reader.h"
#include "Table.h"
#include "UserInputReader.h"
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <iostream>
#include <memory>
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
    std::shared_ptr<ProgramRun> run;
    std::shared_ptr<Reader> reader;
    OutputProcessor out;
    bool live = false;
    bool interactive = false;
    std::thread updater;
    int sortColumn = 0;
    int precision = 3;
    Table relationTable;
    Table ruleTable;
    InputReader linereader;
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
    Tui(std::string filename, bool interactive)
            : run(std::make_shared<ProgramRun>(ProgramRun())),
              reader(std::make_shared<Reader>(filename, run)), out(run), interactive(interactive) {
        // Set a friendlier output size if we're being interacted with directly.
        if (interactive) {
            resultLimit = 20;
        }

        updateDB();
    }

    Tui()
            : run(std::make_shared<ProgramRun>(ProgramRun())), reader(std::make_shared<Reader>(run)),
              out(run), live(true) {
        updateDB();
        top();
        updater = std::thread([this]() {
            // Update the display every 30s. Check for input every 0.5s
            std::chrono::milliseconds interval(30000);
            auto nextUpdateTime = std::chrono::high_resolution_clock::now();
            do {
                if (nextUpdateTime < std::chrono::high_resolution_clock::now()) {
                    // Move up n lines and overwrite the previous top output.
                    std::cout << "\x1b[3D";
                    std::cout << "\x1b[31A";
                    updateDB();
                    top();
                    std::cout << "\x1b[B> ";
                    nextUpdateTime = std::chrono::high_resolution_clock::now() + interval;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
            } while (/*reader->isLive() && */ !linereader.hasReceivedInput());
        });
    }

    ~Tui() {
        if (updater.joinable()) {
            updater.join();
        }
    }

    void runCommand(std::vector<std::string> c) {
        if (linereader.hasReceivedInput()) {
            interactive = true;
            // Input was received, but no command entered so nothing to do
            if (c.empty()) {
                return;
            }
        } else {
            interactive = false;
        }
        if (reader->isLive()) {
            updateDB();
            // remake tables to get new data
            ruleTable = out.getRulTable();

            setupTabCompletion();
        }

        if (c[0] == "top") {
            top();
        } else if (c[0] == "rel") {
            if (c.size() == 2) {
                relRul(c[1]);
            } else if (c.size() == 1) {
                rel(resultLimit);
            } else {
                std::cout << "Invalid parameters to rel command.\n";
            }
        } else if (c[0] == "rul") {
            if (c.size() > 1) {
                if (c.size() == 3 && c[1] == "id") {
                    id(c[2]);
                } else if (c.size() == 2 && c[1] == "id") {
                    ruleIds();
                } else if (c.size() == 2) {
                    verRul(c[1]);
                } else {
                    std::cout << "Invalid parameters to rul command.\n";
                }
            } else {
                rul(resultLimit);
            }
        } else if (c[0] == "graph") {
            if (c.size() == 3 && c[1].find(".") == std::string::npos) {
                iterRel(c[1], c[2]);
            } else if (c.size() == 3 && c[1].at(0) == 'C') {
                iterRul(c[1], c[2]);
            } else if (c.size() == 4 && c[1] == "ver" && c[2].at(0) == 'C') {
                verGraph(c[2], c[3]);
            } else {
                std::cout << "Invalid parameters to graph command.\n";
            }
        } else if (c[0] == "memory") {
            memoryUsage();
        } else if (c[0] == "usage") {
            if (c.size() > 1) {
                if (c[1][0] == 'R') {
                    usageRelation(c[1]);
                } else {
                    usageRule(c[1]);
                }
            } else {
                usage();
            }
        } else if (c[0] == "help") {
            help();
        } else if (c[0] == "limit") {
            if (c.size() == 1) {
                setResultLimit(20000);
            } else {
                try {
                    setResultLimit(std::stoul(c[1]));
                } catch (...) {
                    std::cout << "Invalid parameters to limit command.\n";
                }
            }
        } else if (c[0] == "configuration") {
            configuration();
        } else {
            std::cout << "Unknown command. Use \"help\" for a list of commands.\n";
        }
    }

    void runProf() {
        if (interactive) {
            std::cout << "SouffleProf\n";
            top();
        }

        setupTabCompletion();

        while (true) {
            std::string untrimmedInput = linereader.getInput();
            std::string input = Tools::trimWhitespace(untrimmedInput);

            std::cout << std::endl;
            if (input.empty()) {
                std::cout << "Unknown command. Type help for a list of commands.\n";
                continue;
            }

            linereader.addHistory(input);

            std::vector<std::string> c = Tools::split(input, " ");

            if (c[0] == "q" || c[0] == "quit") {
                quit();
                break;
            } else if (c[0] == "sort") {
                if (c.size() == 2 && std::stoi(c[1]) < 7) {
                    sortColumn = std::stoi(c[1]);
                } else {
                    std::cout << "Invalid column, please select a number between 0 and 6.\n";
                }
            } else {
                runCommand(c);
            }
        }
    }

    void quit() {
        if (updater.joinable()) {
            updater.join();
        }
    }

    static void help() {
        std::cout << "\nAvailable profiling commands:" << std::endl;
        std::printf("  %-30s%-5s %s\n", "rel", "-", "display relation table.");
        std::printf("  %-30s%-5s %s\n", "rel <relation id>", "-", "display all rules of a given relation.");
        std::printf("  %-30s%-5s %s\n", "rul", "-", "display rule table");
        std::printf("  %-30s%-5s %s\n", "rul <rule id>", "-", "display all version of given rule.");
        std::printf("  %-30s%-5s %s\n", "rul id", "-", "display all rules names and ids.");
        std::printf(
                "  %-30s%-5s %s\n", "rul id <rule id>", "-", "display the rule name for the given rule id.");
        std::printf("  %-30s%-5s %s\n", "graph <relation id> <type>", "-",
                "graph a relation by type: (tot_t/copy_t/tuples).");
        std::printf("  %-30s%-5s %s\n", "graph <rule id> <type>", "-",
                "graph recursive(C) rule by type(tot_t/tuples).");
        std::printf("  %-30s%-5s %s\n", "graph ver <rule id> <type>", "-",
                "graph recursive(C) rule versions by type(tot_t/copy_t/tuples).");
        std::printf("  %-30s%-5s %s\n", "top", "-", "display top-level summary of program run.");
        std::printf("  %-30s%-5s %s\n", "configuration", "-", "display configuration settings for this run.");
        std::printf("  %-30s%-5s %s\n", "usage [relation id|rule id]", "-",
                "display CPU usage graphs for a relation or rule.");
        std::printf("  %-30s%-5s %s\n", "memory", "-", "display memory usage.");
        std::printf("  %-30s%-5s %s\n", "help", "-", "print this.");

        std::cout << "\nInteractive mode only commands:" << std::endl;
        std::printf("  %-30s%-5s %s\n", "limit <row count>", "-", "limit number of results shown.");
        std::printf("  %-30s%-5s %s\n", "sort <col number>", "-", "sort tables by given column number.");
        std::printf("  %-30s%-5s %s\n", "q", "-", "exit program.");
    }

    /** Print usage statistics during evaluation of a relation */
    void usageRelation(std::string id) {
        const auto rel = run->getRelationById(id);

        if (rel == nullptr) {
            std::cout << "*Relation does not exist.\n";
            return;
        }

        usage(rel->getEndtime(), rel->getStarttime());
    }

    /** Check if a given id is a valid relation id. */
    bool isValidRelationId(std::string relationId) {
        return run->getRelationById(relationId) == nullptr;
    }

    /** Check if a given id is a valid rule id. */
    bool isValidRuleId(std::string id) {
        auto relation = run->getRelationById(getRelationId(id));

        // Check for invalid relation name
        if (relation == nullptr) {
            return false;
        }

        // Rules are indexed by srclocator, so we need to scan
        for (auto& rulePair : relation->getRuleMap()) {
            if (rulePair.second->getId() == id) {
                return true;
            }
        }
        return false;
    }

    /** Extract relation id from a rule id. */
    std::string getRelationId(std::string ruleId) {
        if (ruleId.size() < 4) {
            return "";
        }
        auto pos = ruleId.find_first_of('.');
        if (pos < 2) {
            return "";
        }

        return "R" + ruleId.substr(1, pos - 1);
    }

    /** Print usage statistics during evaluation of a rule. */
    void usageRule(std::string ruleId) {
        if (!isValidRuleId(ruleId)) {
            std::cout << "Rule does not exist.\n";
            return;
        }
        auto relation = run->getRelationById(getRelationId(ruleId));
        if (relation == nullptr) {
            std::cout << "Relation ceased to exist. Odd." << std::endl;
            return;
        }
        for (auto& rulePair : relation->getRuleMap()) {
            if (rulePair.second->getId() == ruleId) {
                usage(rulePair.second->getEndtime(), rulePair.second->getStarttime());
                return;
            }
        }

        std::cout << "Rule ceased to exist. Odd." << std::endl;
    }

    /** Get overall usage statistics.
     *
     * @param width maximum number of increments to return.
     * @return set of Usages of up to width in size
     */
    std::set<Usage> getUsageStats(size_t width = size_t(-1)) {
        std::set<Usage> usages;
        DirectoryEntry* usageStats = dynamic_cast<DirectoryEntry*>(
                ProfileEventSingleton::instance().getDB().lookupEntry({"program", "usage", "timepoint"}));
        if (usageStats == nullptr || usageStats->getKeys().size() < 2) {
            return usages;
        }
        std::chrono::microseconds endTime{};
        std::chrono::microseconds startTime{};
        std::chrono::microseconds timeStep{};
        // Translate the string ordered text usage stats to a time ordered binary form.
        std::set<Usage> allUsages;
        for (auto& currentKey : usageStats->getKeys()) {
            Usage currentUsage{};
            uint64_t cur = std::stoul(currentKey);
            currentUsage.time = std::chrono::duration<uint64_t, std::micro>(cur);
            cur = dynamic_cast<SizeEntry*>(
                    usageStats->readDirectoryEntry(currentKey)->readEntry("systemtime"))
                          ->getSize();
            currentUsage.systemtime = std::chrono::duration<uint64_t, std::micro>(cur);
            cur = dynamic_cast<SizeEntry*>(usageStats->readDirectoryEntry(currentKey)->readEntry("usertime"))
                          ->getSize();
            currentUsage.usertime = std::chrono::duration<uint64_t, std::micro>(cur);
            currentUsage.maxRSS =
                    dynamic_cast<SizeEntry*>(usageStats->readDirectoryEntry(currentKey)->readEntry("maxRSS"))
                            ->getSize();

            // Duplicate times are possible
            if (allUsages.find(currentUsage) != allUsages.end()) {
                auto& existing = *allUsages.find(currentUsage);
                currentUsage.systemtime = std::max(existing.systemtime, currentUsage.systemtime);
                currentUsage.usertime = std::max(existing.usertime, currentUsage.usertime);
                currentUsage.maxRSS = std::max(existing.maxRSS, currentUsage.maxRSS);
                allUsages.erase(currentUsage);
            }
            allUsages.insert(currentUsage);
        }

        // cpu times aren't quite recorded in a monotonic way, so skip the invalid ones.
        for (auto it = ++allUsages.begin(); it != allUsages.end(); ++it) {
            auto previous = std::prev(it);
            if (it->usertime < previous->usertime || it->systemtime < previous->systemtime ||
                    it->time == previous->time) {
                it = allUsages.erase(it);
                --it;
            }
        }

        // Extract our overall stats
        startTime = allUsages.begin()->time;
        endTime = allUsages.rbegin()->time;

        // If we don't have enough records, just return what we can
        if (allUsages.size() < width) {
            return allUsages;
        }

        timeStep = (endTime - startTime) / width;

        // Store the timepoints we need for the graph
        for (uint32_t i = 1; i <= width; ++i) {
            auto it = allUsages.upper_bound(Usage{startTime + timeStep * i, 0, {}, {}});
            if (it != allUsages.begin()) {
                --it;
            }
            usages.insert(*it);
        }

        return usages;
    }

    void usage(uint32_t height = 20) {
        usage({}, {}, height);
    }

    void usage(std::chrono::microseconds endTime, std::chrono::microseconds startTime, uint32_t height = 20) {
        uint32_t width = getTermWidth() - 8;

        std::set<Usage> usages = getUsageStats(width);

        if (usages.size() < 2) {
            for (uint8_t i = 0; i < height + 2; ++i) {
                std::cout << std::endl;
            }
            std::cout << "Insufficient data for usage statistics." << std::endl;
            return;
        }

        double maxIntervalUsage = 0;

        // Extract our overall stats
        if (startTime.count() == 0) {
            startTime = usages.begin()->time;
        }
        if (endTime.count() == 0) {
            endTime = usages.rbegin()->time;
        }

        if (usages.size() < width) {
            width = usages.size();
        }

        // Find maximum so we can normalise the graph
        Usage previousUsage{{}, 0, {}, {}};
        for (auto& currentUsage : usages) {
            double usageDiff = (currentUsage.systemtime - previousUsage.systemtime + currentUsage.usertime -
                                previousUsage.usertime)
                                       .count();
            usageDiff /= (currentUsage.time - previousUsage.time).count();
            if (usageDiff > maxIntervalUsage) {
                maxIntervalUsage = usageDiff;
            }

            previousUsage = currentUsage;
        }

        double intervalUsagePercent = 100.0 * maxIntervalUsage;
        std::printf("%11s\n", "cpu total");
        std::printf("%11s\n", Tools::formatTime(usages.rbegin()->usertime).c_str());

        // Add columns to the graph
        char grid[height][width];
        for (uint32_t i = 0; i < height; ++i) {
            for (uint32_t j = 0; j < width; ++j) {
                grid[i][j] = ' ';
            }
        }

        previousUsage = {{}, 0, {}, {}};
        uint32_t col = 0;
        for (const Usage& currentUsage : usages) {
            uint64_t curHeight = 0;
            uint64_t curSystemHeight = 0;
            // Usage may be 0
            if (maxIntervalUsage != 0) {
                curHeight = (currentUsage.systemtime - previousUsage.systemtime + currentUsage.usertime -
                             previousUsage.usertime)
                                    .count();
                curHeight /= (currentUsage.time - previousUsage.time).count();
                curHeight *= height / maxIntervalUsage;

                curSystemHeight = (currentUsage.systemtime - previousUsage.systemtime).count();
                curSystemHeight /= (currentUsage.time - previousUsage.time).count();
                curSystemHeight *= height / maxIntervalUsage;
            }
            for (uint32_t row = 0; row < curHeight; ++row) {
                grid[row][col] = '*';
            }
            for (uint32_t row = curHeight - curSystemHeight; row < curHeight; ++row) {
                grid[row][col] = '+';
            }
            previousUsage = currentUsage;
            ++col;
        }

        // Print array
        for (int32_t row = height - 1; row >= 0; --row) {
            printf("%6d%% ", uint32_t(intervalUsagePercent * (row + 1) / height));
            for (uint32_t col = 0; col < width; ++col) {
                std::cout << grid[row][col];
            }
            std::cout << std::endl;
        }
        for (uint32_t col = 0; col < 8; ++col) {
            std::cout << ' ';
        }
        for (uint32_t col = 0; col < width; ++col) {
            std::cout << '-';
        }
        std::cout << std::endl;
    }

    void memoryUsage(uint32_t height = 20) {
        memoryUsage({}, {}, height);
    }

    void memoryUsage(std::chrono::microseconds /* endTime */, std::chrono::microseconds /* startTime */,
            uint32_t height = 20) {
        uint32_t width = getTermWidth() - 8;
        uint64_t maxMaxRSS = 0;

        std::set<Usage> usages = getUsageStats(width);
        char grid[height][width];
        for (uint32_t i = 0; i < height; ++i) {
            for (uint32_t j = 0; j < width; ++j) {
                grid[i][j] = ' ';
            }
        }

        for (auto& usage : usages) {
            maxMaxRSS = std::max(maxMaxRSS, usage.maxRSS);
        }
        size_t col = 0;
        for (const Usage& currentUsage : usages) {
            uint64_t curHeight = height * currentUsage.maxRSS / maxMaxRSS;
            for (uint32_t row = 0; row < curHeight; ++row) {
                grid[row][col] = '*';
            }
            ++col;
        }

        // Print array
        for (int32_t row = height - 1; row >= 0; --row) {
            printf("%6s ", Tools::formatMemory(maxMaxRSS * (row + 1) / height).c_str());
            for (uint32_t col = 0; col < width; ++col) {
                std::cout << grid[row][col];
            }
            std::cout << std::endl;
        }
        for (uint32_t col = 0; col < 8; ++col) {
            std::cout << ' ';
        }
        for (uint32_t col = 0; col < width; ++col) {
            std::cout << '-';
        }
        std::cout << std::endl;
    }
    void setupTabCompletion() {
        linereader.clearTabCompletion();

        linereader.appendTabCompletion("rel");
        linereader.appendTabCompletion("rul");
        linereader.appendTabCompletion("rul id");
        linereader.appendTabCompletion("graph ");
        linereader.appendTabCompletion("top");
        linereader.appendTabCompletion("help");
        linereader.appendTabCompletion("usage");
        linereader.appendTabCompletion("limit ");
        linereader.appendTabCompletion("memory");
        linereader.appendTabCompletion("configuration");

        // add rel tab completes after the rest so users can see all commands first
        for (auto& row : Tools::formatTable(relationTable, precision)) {
            linereader.appendTabCompletion("rel " + row[5]);
            linereader.appendTabCompletion("graph " + row[5] + " tot_t");
            linereader.appendTabCompletion("graph " + row[5] + " copy_t");
            linereader.appendTabCompletion("graph " + row[5] + " tuples");
            linereader.appendTabCompletion("usage " + row[5]);
        }
    }

    void configuration() {
        std::cout << "Configuration" << '\n';
        printf("%30s      %s", "Key", "Value\n\n");
        for (auto& kvp :
                ProfileEventSingleton::instance().getDB().getStringMap({"program", "configuration"})) {
            if (kvp.first == "") {
                printf("%30s      %s\n", "Datalog input file", kvp.second.c_str());
                continue;
            }
            printf("%30s      %s\n", kvp.first.c_str(), kvp.second.c_str());
        }
        std::cout << std::endl;
    }

    /** Show summary statistics and progress */
    void top() {
        auto* totalRelationsEntry =
                dynamic_cast<TextEntry*>(ProfileEventSingleton::instance().getDB().lookupEntry(
                        {"program", "configuration", "relationCount"}));
        auto* totalRulesEntry =
                dynamic_cast<TextEntry*>(ProfileEventSingleton::instance().getDB().lookupEntry(
                        {"program", "configuration", "ruleCount"}));
        size_t totalRelations = 0;
        if (totalRelationsEntry != nullptr) {
            totalRelations = std::stoul(totalRelationsEntry->getText());
        } else {
            totalRelations = run->getRelationMap().size();
        }
        size_t totalRules = 0;
        if (totalRulesEntry != nullptr) {
            totalRules = std::stoul(totalRulesEntry->getText());
        } else {
            // TODO: remove ruletable references
            totalRules = ruleTable.getRows().size();
        }
        std::printf("%11s%10s%10s%10s%10s%20s\n", "runtime", "loadtime", "savetime", "relations", "rules",
                "tuples generated");

        std::printf("%11s%10s%10s%10s%10s%14s\n", run->getRuntime().c_str(),
                Tools::formatTime(run->getTotalLoadtime()).c_str(),
                Tools::formatTime(run->getTotalSavetime()).c_str(),
                Tools::formatNum(0, totalRelations).c_str(), Tools::formatNum(0, totalRules).c_str(),
                Tools::formatNum(precision, run->getTotalSize()).c_str());

        // Progress bar
        // Determine number of relations processed
        size_t processedRelations = run->getRelationMap().size();
        size_t screenWidth = getTermWidth() - 10;
        if (live && totalRelationsEntry != nullptr) {
            std::cout << "Progress ";
            for (size_t i = 0; i < screenWidth; ++i) {
                if (screenWidth * processedRelations / totalRelations > i) {
                    std::cout << '#';
                } else {
                    std::cout << '_';
                }
            }
        }
        std::cout << std::endl;

        std::cout << "Slowest relations to fully evaluate\n";
        rel(3, false);
        for (size_t i = run->getRelationsByFrequency().size(); i < 3; ++i) {
            std::cout << "\n";
        }
        std::cout << "Slowest rules to fully evaluate\n";
        rul(3, false);
        // TODO: remove ruletable references
        for (size_t i = ruleTable.getRows().size(); i < 3; ++i) {
            std::cout << "\n";
        }

        usage(10);
    }

    void setResultLimit(size_t limit) {
        resultLimit = limit;
    }

    /**
     * @brief Print the top relations by frequency
     *
     * @param limit how many relations to print
     * @param showlimit set true to show how many relations were not printed
     *
     */
    void rel(size_t limit, bool showLimit = true) {
        std::cout << " ----- Relation Table -----\n";
        relSummaryHeaders();
        auto relations = run->getRelationsByFrequency();
        for (auto relation : relations) {
            if (limit-- == 0) {
                if (showLimit) {
                    std::cout << (relations.size() - resultLimit) << " rows not shown" << std::endl;
                }
                break;
            }
            relSummary(relation);
        }
    }

    /**
     * @brief Print the top rules by frequency
     *
     * @param limit how many rules to print
     * @param showlimit set true to show how many rules were not printed
     *
     */
    void rul(size_t limit, bool showLimit = true) {
        auto rules = getAllRules();
        std::cout << "  ----- Rule Table -----\n";
        std::printf(
                "%8s%8s%8s%8s%8s%8s %s\n\n", "TOT_T", "NREC_T", "REC_T", "TUPLES", "TUP/s", "ID", "RELATION");
        size_t count = 0;
        for (auto& rulePair : rules) {
            if (++count > limit) {
                if (showLimit) {
                    std::cout << (rules.size() - resultLimit) << " rows not shown" << std::endl;
                }
                break;
            }
            auto& rule = rulePair.second;

            std::cout << std::setw(8) << Tools::formatTime(rule->getRuntime());
            std::cout << std::setw(8) << Tools::formatTime(rule->getNonrecursiveRuntime());
            std::cout << std::setw(8) << Tools::formatTime(rule->getRecursiveRuntime());
            std::cout << std::setw(8) << Tools::formatNum(rule->size());
            if (rule->getRuntime().count() != 0) {
                double tuplesPerSecond = rule->size() / (rule->getRuntime().count() / 1000000.0);
                std::cout << std::setw(8) << Tools::formatNum(tuplesPerSecond);
            } else {
                std::cout << std::setw(8) << "-";
            }
            std::cout << std::setw(8) << rule->getId() << std::endl;
        }
    }

    /** Print out all rule id to name mappings */
    void ruleIds() {
        std::map<std::string, std::string> ruleIdMap;
        for (auto& relationPair : run->getRelationMap()) {
            for (auto& rule : relationPair.second->getRuleTotals()) {
                ruleIdMap[rule->getId()] = Tools::cleanString(rule->getName());
            }
        }

        std::printf("%7s%2s%s\n\n", "1ID", "", "NAME");
        for (auto rulePair : ruleIdMap) {
            std::cout << std::setw(7) << rulePair.first << "  " << rulePair.second << std::endl;
        }
    }

    /** Print out all rule id to name mappings */
    void id(std::string id) {
        std::map<std::string, std::string> ruleIdMap;
        for (auto& relationPair : run->getRelationMap()) {
            for (auto& rule : relationPair.second->getRuleTotals()) {
                ruleIdMap[rule->getId()] = Tools::cleanString(rule->getName());
            }
        }

        std::printf("%7s%2s%s\n\n", "ID", "", "NAME");
        for (auto rulePair : ruleIdMap) {
            if (rulePair.first == id) {
                std::cout << std::setw(7) << rulePair.first << "  " << rulePair.second << std::endl;
                break;
            }
        }
    }

    /** Find a rule using the given id, whether norecursive or recursive. */
    std::shared_ptr<Rule> getRuleById(std::string ruleId) {
        auto relation = run->getRelationById(getRelationId(ruleId));
        if (relation == nullptr) {
            std::cout << "Relation for id not found" << std::endl;
            return nullptr;
        }

        for (auto rulePair : relation->getRuleMap()) {
            if (rulePair.second->getId() == ruleId) {
                return rulePair.second;
            }
        }
        for (auto& iteration : relation->getIterations()) {
            for (auto& rulePair : iteration->getRules()) {
                if (rulePair.second->getId() == ruleId) {
                    return rulePair.second;
                }
            }
        }

        return nullptr;
    }

    std::map<std::string, std::shared_ptr<SummedRule>> getAllRules() {
        std::map<std::string, std::shared_ptr<SummedRule>> rules;
        for (auto& relationPair : run->getRelationMap()) {
            auto& relation = relationPair.second;
            for (auto& rule : relation->getRuleTotals()) {
                rules.try_emplace(
                        rule->getId(), std::make_shared<SummedRule>(rule->getName(), rule->getId()));
                *rules[rule->getId()] += *rule;
            }
        }
        return rules;
    }

    void relSummaryHeaders() const {
        std::printf("%8s%8s%8s%8s%8s%8s%8s%8s%8s%6s %s\n\n", "TOT_T", "NREC_T", "REC_T", "COPY_T", "LOAD_T",
                "SAVE_T", "TUPLES", "READS", "TUP/s", "ID", "NAME");
    }

    void relSummary(std::shared_ptr<Relation> relation) const {
        std::cout << std::setw(8) << Tools::formatTime(relation->getRunTime());
        std::cout << std::setw(8) << Tools::formatTime(relation->getNonRecTime());
        std::cout << std::setw(8) << Tools::formatTime(relation->getRecTime());
        std::cout << std::setw(8) << Tools::formatTime(relation->getCopyTime());
        std::cout << std::setw(8) << Tools::formatTime(relation->getLoadtime());
        std::cout << std::setw(8) << Tools::formatTime(relation->getSavetime());
        std::cout << std::setw(8) << Tools::formatNum(precision, relation->size());
        std::cout << std::setw(8) << Tools::formatNum(precision, relation->getReads());
        std::cout << std::setw(8)
                  << Tools::formatNum(
                             precision, relation->size() / (relation->getRunTime().count() / 1000000.0));
        std::cout << std::setw(6) << relation->getId();
        std::cout << " " << relation->getName();
        std::cout << std::endl;
    }

    /** Print out the rules of a Relation */
    void relRul(std::string str) {
        std::shared_ptr<Relation> relation = run->getRelation(str);
        if (relation == nullptr) {
            relation = run->getRelationById(str);
        }
        if (relation == nullptr) {
            std::cout << "Unknown relation" << std::endl;
            return;
        }

        std::cout << "  ----- Rules of a Relation -----\n";
        std::printf("%8s%8s%8s%8s%8s\n\n", "TOT_T", "NREC_T", "REC_T", "TUPLES", "ID");

        // Relation summary
        std::cout << std::setw(8) << Tools::formatTime(relation->getRunTime());
        std::cout << std::setw(8) << Tools::formatTime(relation->getNonRecTime());
        std::cout << std::setw(8) << Tools::formatTime(relation->getRecTime());
        std::cout << std::setw(8) << Tools::formatNum(precision, relation->size());
        std::cout << std::setw(6) << relation->getId();
        std::cout << " " << relation->getName();
        std::cout << std::endl;

        // Rule summary
        std::cout << " ---------------------------------------------------------\n";
        for (auto& rule : relation->getRuleTotals()) {
            std::cout << std::setw(8) << Tools::formatTime(rule->getRuntime());
            std::cout << std::setw(8) << Tools::formatTime(rule->getNonrecursiveRuntime());
            std::cout << std::setw(8) << Tools::formatTime(rule->getRecursiveRuntime());
            std::cout << std::setw(8) << Tools::formatNum(rule->size());
            std::cout << std::setw(8) << rule->getId();
            std::cout << std::endl;
        }

        std::cout << "\nSrc locator: " << relation->getLocator() << "\n\n";

        // Map rule ID to rule
        for (auto& rule : relation->getRuleTotals()) {
            std::cout << std::setw(8) << rule->getId();
            std::cout << "  " << Tools::cleanString(rule->getName());
            std::cout << std::endl;
        }
    }

    void verRul(std::string str) {
        if (str.find(".") == std::string::npos) {
            std::cout << "Rule does not exist\n";
            return;
        }
        std::vector<std::string> part = Tools::split(str, ".");
        std::string strRel = "R" + part[0].substr(1);

        Table versionTable = out.getVersions(strRel, str);
        versionTable.sort(sortColumn);

        // TODO: remove ruletable references
        ruleTable.sort(sortColumn);  // why isnt it sorted in the original java?!?

        std::vector<std::vector<std::string>> formattedRuleTable = Tools::formatTable(ruleTable, precision);

        bool found = false;
        std::string ruleName;
        std::string srcLocator;
        // Check that the rule exists, and print it out if so.
        for (auto& row : formattedRuleTable) {
            if (row[6] == str) {
                std::cout << row[5] << std::endl;
                found = true;
                ruleName = row[5];
                srcLocator = row[10];
            }
        }

        // If the rule exists, print out the source locator.
        if (found) {
            if (versionTable.rows.size() > 0) {
                if (versionTable.rows[0]->cells[9] != nullptr) {
                    std::cout << "Src locator-: " << (*versionTable.rows[0]).getStringValue(9) << "\n\n";
                } else {
                    std::cout << "Src locator-: -\n\n";
                }
            } else if (formattedRuleTable.size() > 0) {
                std::cout << "Src locator-: " << formattedRuleTable[0][10] << "\n\n";
            }
        }

        // Print out the versions of this rule.
        std::cout << "  ----- Rule Versions Table -----\n";
        std::printf("%8s%8s%8s%16s%6s\n\n", "TOT_T", "NREC_T", "REC_T", "TUPLES", "VER");
        for (auto& row : formattedRuleTable) {
            if (row[6] == str) {
                std::printf("%8s%8s%8s%16s%6s\n", row[0].c_str(), row[1].c_str(), row[2].c_str(),
                        row[4].c_str(), "");
            }
        }
        std::cout << "   ---------------------------------------------\n";
        for (auto& _row : versionTable.rows) {
            Row row = *_row;

            std::printf("%8s%8s%8s%16s%6s\n", row.valueToString(0, precision).c_str(),
                    row.valueToString(1, precision).c_str(), row.valueToString(2, precision).c_str(),
                    row.valueToString(4, precision).c_str(), row.valueToString(8, precision).c_str());
            Table atom_table = out.getVersionAtoms(strRel, srcLocator, row.getLongValue(8));
            verAtoms(atom_table);
        }

        if (!versionTable.rows.empty()) {
            return;
        }

        Table atom_table = out.getAtomTable(strRel, str);
        verAtoms(atom_table, ruleName);
    }

    /** Print graph of total time, copy time, or tuples generated per iteration of a relation .*/
    void iterRel(std::string relationId, std::string col) {
        auto relation = run->getRelationById(relationId);
        if (relation == nullptr) {
            return;
        }

        std::cout << std::setw(4) << relation->getId() << "  " << relation->getName() << "\n\n";

        auto iter = relation->getIterations();
        if (col == "tot_t") {
            std::vector<std::chrono::microseconds> list;
            for (auto& i : iter) {
                list.emplace_back(i->getRuntime());
            }
            std::cout << "No     Runtime\n\n";
            graphByTime(list);
        } else if (col == "copy_t") {
            std::vector<std::chrono::microseconds> list;
            for (auto& i : iter) {
                list.emplace_back(i->getCopytime());
            }
            std::cout << "No     Copytime\n\n";
            graphByTime(list);
        } else if (col == "tuples") {
            std::vector<size_t> list;
            for (auto& i : iter) {
                list.emplace_back(i->size());
            }
            std::cout << "No     Tuples\n\n";
            graphBySize(list);
        }
    }

    /** Print graph of total time or tuples generated per iteration of a rule. */
    void iterRul(std::string c, std::string col) {
        auto ruleId = c;

        auto rule = getRuleById(ruleId);
        if (rule == nullptr) {
            std::cout << "Rule not found" << std::endl;
            return;
        }
        auto relation = run->getRelationById(getRelationId(ruleId));
        if (relation == nullptr) {
            std::cout << "Relation not found" << std::endl;
            return;
        }

        std::cout << std::setw(6) << rule->getId() << "  " << rule->getName() << "\n\n";
        if (col == "tot_t") {
            std::vector<std::chrono::microseconds> list;
            for (auto& iteration : relation->getIterations()) {
                bool add = false;
                std::chrono::microseconds totalTime{};
                for (auto& rulePair : iteration->getRules()) {
                    if (rulePair.second->getId() == ruleId) {
                        totalTime += rulePair.second->getRuntime();
                        add = true;
                    }
                }
                if (add) {
                    list.emplace_back(totalTime);
                }
            }
            std::printf("%4s   %s\n\n", "NO", "RUNTIME");
            graphByTime(list);
        } else if (col == "tuples") {
            std::vector<size_t> list;
            for (auto& iteration : relation->getIterations()) {
                bool add = false;
                size_t totalSize = 0L;
                for (auto& rulePair : iteration->getRules()) {
                    if (rulePair.second->getId() == ruleId) {
                        totalSize += rulePair.second->size();
                        add = true;
                    }
                }
                if (add) {
                    list.emplace_back(totalSize);
                }
            }
            std::printf("%4s   %s\n\n", "NO", "TUPLES");
            graphBySize(list);
        }
    }

    void verGraph(std::string c, std::string col) {
        if (c.find('.') == std::string::npos) {
            std::cout << "Rule does not exist";
            return;
        }

        std::vector<std::string> part = Tools::split(c, ".");
        std::string strRel = "R" + part[0].substr(1);

        Table versionTable = out.getVersions(strRel, c);
        std::printf("%6s%2s%s\n\n", (*versionTable.rows[0]).valueToString(6, 0).c_str(), "",
                (*versionTable.rows[0]).valueToString(5, 0).c_str());
        if (col == "tot_t") {
            std::vector<std::chrono::microseconds> list;
            for (auto& row : versionTable.rows) {
                list.emplace_back((*row).getTimeValue(0));
            }
            std::printf("%4s   %s\n\n", "NO", "RUNTIME");
            graphByTime(list);
        } else if (col == "copy_t") {
            std::vector<std::chrono::microseconds> list;
            for (auto& row : versionTable.rows) {
                list.emplace_back((*row).getTimeValue(3));
            }
            std::printf("%4s   %s\n\n", "NO", "COPYTIME");
            graphByTime(list);
        } else if (col == "tuples") {
            std::vector<size_t> list;
            for (auto& row : versionTable.rows) {
                list.emplace_back((*row).getLongValue(4));
            }
            std::printf("%4s   %s\n\n", "NO", "TUPLES");
            graphBySize(list);
        }
    }

    void graphByTime(std::vector<std::chrono::microseconds> list) {
        std::chrono::microseconds max{};
        for (auto& d : list) {
            if (d > max) {
                max = d;
            }
        }

        std::sort(list.begin(), list.end());
        std::reverse(list.begin(), list.end());
        int i = 0;
        for (auto& d : list) {
            uint32_t len = 67.0 * d.count() / max.count();
            std::string bar = "";
            for (uint32_t j = 0; j < len; j++) {
                bar += "*";
            }

            std::printf("%4d %10.8f | %s\n", i++, (d.count() / 1000000.0), bar.c_str());
        }
    }

    void graphBySize(std::vector<size_t> list) {
        size_t max = 0;
        for (auto& l : list) {
            if (l > max) {
                max = l;
            }
        }
        std::sort(list.begin(), list.end());
        std::reverse(list.begin(), list.end());
        uint32_t i = 0;
        for (auto& l : list) {
            size_t len = max == 0 ? 0 : 64.0 * l / max;
            std::string bar = "";
            for (uint32_t j = 0; j < len; j++) {
                bar += "*";
            }

            std::printf("%4d %8s | %s\n", i++, Tools::formatNum(precision, l).c_str(), bar.c_str());
        }
    }

protected:
    void verAtoms(Table& atomTable, const std::string& ruleName = "") {
        // If there are no subrules then just print out any atoms found
        // If we do find subrules, then label atoms with their subrules.
        if (atomTable.rows.empty()) {
            return;
        }
        bool firstRun = true;
        std::string lastRule = ruleName;
        for (auto& _row : atomTable.rows) {
            Row& row = *_row;
            std::string rule = row.valueToString(0, precision);
            if (rule != lastRule) {
                lastRule = rule;
                std::cout << "     " << row.valueToString(0, precision) << std::endl;
                firstRun = true;
            }
            if (firstRun) {
                std::printf("      %-16s%-16s%s\n", "FREQ", "RELSIZE", "ATOM");
                firstRun = false;
            }
            std::string relationName = row.getStringValue(1);
            relationName = relationName.substr(0, relationName.find('('));
            auto relation = run->getRelation(relationName);
            std::string relationSize = relation == nullptr ? "--" : std::to_string(relation->size());
            std::printf("      %-16s%-16s%s\n", row.valueToString(3, precision).c_str(), relationSize.c_str(),
                    row.getStringValue(1).c_str());
        }
        std::cout << '\n';
    }
    void updateDB() {
        reader->processFile();
        ruleTable = out.getRulTable();
    }

    uint32_t getTermWidth() {
        struct winsize w {};
        ioctl(0, TIOCGWINSZ, &w);
        uint32_t width = w.ws_col > 0 ? w.ws_col : 80;
        return width;
    }
};

}  // namespace profile
}  // namespace souffle
