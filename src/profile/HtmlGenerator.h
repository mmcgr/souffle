/*
 * Souffle - A Datalog Compiler
 * Copyright (c) 2016, The Souffle Developers. All rights reserved
 * Licensed under the Universal Permissive License v 1.0 as shown at:
 * - https://opensource.org/licenses/UPL
 * - <souffle root>/licenses/SOUFFLE-UPL.txt
 */

#pragma once

#include "../ProfileEvent.h"
#include "OutputProcessor.h"
#include "ProgramRun.h"
#include "Reader.h"
#include "htmlCssChartist.h"
#include "htmlCssStyle.h"
#include "htmlJsChartistMin.h"
#include "htmlJsChartistPlugin.h"
#include "htmlJsMain.h"
#include "htmlJsTableSort.h"
#include "htmlJsUtil.h"
#include "htmlMain.h"
#include "utility/FileUtil.h"
#include <sstream>
#include <string>

namespace souffle {
namespace profile {

/*
 * Class linking the html, css, and js into one html file
 * so that a data variable can be inserted in the middle of the two strings and written to a file.
 *
 */
class HtmlGenerator {
public:
    HtmlGenerator(std::string dbFilename) : run(std::make_shared<ProgramRun>()), out(run) {
        Reader reader(dbFilename, run);
        reader.processFile();
        ruleTable = out.getRulTable();
    }

    void outputHtml() {
        std::string path = "profiler_html/";
        makeDir(path);
        std::string filename = genFilename(path);
        outputHtml(filename);
    }

    void outputHtml(std::string filename) {
        std::cout << "SouffleProf\n";
        std::cout << "Generating HTML files...\n";

        checkFile(filename);
        std::ofstream outfile(filename);

        outfile << getHtml(genJson());

        std::cout << "file output to: " << filename << std::endl;
    }

protected:
    struct Usage {
        std::chrono::microseconds time;
        uint64_t maxRSS;
        std::chrono::microseconds systemtime;
        std::chrono::microseconds usertime;
        bool operator<(const Usage& other) const {
            return time < other.time;
        }
    };

    std::shared_ptr<ProgramRun> run;
    OutputProcessor out;
    Table ruleTable;
    // FIXME: refactor to use run->getRelation*
    Table relationTable;

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

    void checkFile(std::string filename) {
        std::string path = dirName(filename);
        makeDir(path);
    }

    void makeDir(std::string path) {
        // First check if path already exists
        if (existDir(path)) {
            return;
        }

        mode_t nMode = 0733;  // UNIX style permissions
        int nError = mkdir(path.c_str(), nMode);
        if (nError != 0) {
            std::cerr << "directory " << path << " could not be created.";
            exit(EXIT_FAILURE);
        }
    }

    std::string genFilename(std::string path) {
        std::string filetype = ".html";
        std::string newFile;

        int i = 0;
        do {
            ++i;
            newFile = path + std::to_string(i) + filetype;
        } while (existFile(newFile));

        return newFile;
    }

    std::string getHtml(std::string json) {
        return getFirstHalf() + json + getSecondHalf();
    }

    static std::string getFirstHalf() {
        std::stringstream ss;
        ss << html::htmlHeadTop << HtmlGenerator::wrapCss(html::cssChartist) << wrapCss(html::cssStyle)
           << html::htmlHeadBottom << html::htmlBodyTop << "<script>data=";
        return ss.str();
    }
    static std::string getSecondHalf() {
        std::stringstream ss;
        ss << "</script>" << wrapJs(html::jsTableSort) << wrapJs(html::jsChartistMin)
           << wrapJs(html::jsChartistPlugin) << wrapJs(html::jsUtil) << wrapJs(html::jsMain)
           << html::htmlBodyBottom;
        return ss.str();
    }
    static std::string wrapCss(const std::string& css) {
        return "<style>" + css + "</style>";
    }
    static std::string wrapJs(const std::string& js) {
        return "<script>" + js + "</script>";
    }

    std::stringstream& genJsonTop(std::stringstream& ss) {
        auto beginTime = run->getStarttime();
        auto endTime = run->getEndtime();
        ss << R"_({"top":[)_" << (endTime - beginTime).count() / 1000000.0 << "," << run->getTotalSize()
           << "," << run->getTotalLoadtime().count() / 1000000.0 << ","
           << run->getTotalSavetime().count() / 1000000.0 << "]";
        return ss;
    }

    std::stringstream& genJsonRelations(std::stringstream& ss, const std::string& name, size_t maxRows) {
        auto comma = [&ss](bool& first, const std::string& delimiter = ", ") {
            if (!first) {
                ss << delimiter;
            } else {
                first = false;
            }
        };

        ss << '"' << name << R"_(":{)_";
        bool firstRow = true;
        auto rows = relationTable.getRows();
        std::stable_sort(rows.begin(), rows.end(), [](std::shared_ptr<Row> left, std::shared_ptr<Row> right) {
            return left->getDoubleValue(0) > right->getDoubleValue(0);
        });
        maxRows = std::min(rows.size(), maxRows);

        for (size_t i = 0; i < maxRows; ++i) {
            comma(firstRow, ",\n");

            Row& row = *rows[i];
            ss << '"' << row.valueToString(6, 0) << R"_(": [)_";
            ss << '"' << Tools::cleanJsonOut(row.valueToString(5, 0)) << R"_(", )_";
            ss << '"' << Tools::cleanJsonOut(row.valueToString(6, 0)) << R"_(", )_";
            ss << row.getDoubleValue(0) << ", ";
            ss << row.getDoubleValue(1) << ", ";
            ss << row.getDoubleValue(2) << ", ";
            ss << row.getDoubleValue(3) << ", ";
            ss << row.getLongValue(4) << ", ";
            ss << row.getLongValue(12) << ", ";
            ss << '"' << Tools::cleanJsonOut(row.valueToString(7, 0)) << R"_(", [)_";

            bool firstCol = true;
            for (auto& _rel_row : ruleTable.getRows()) {
                Row rel_row = *_rel_row;
                if (rel_row.valueToString(7, 0) == row.valueToString(5, 0)) {
                    comma(firstCol);
                    ss << '"' << rel_row.valueToString(6, 0) << '"';
                }
            }
            ss << "], ";
            std::vector<std::shared_ptr<Iteration>> iter =
                    run->getRelation(row.valueToString(5, 0))->getIterations();
            ss << R"_({"tot_t": [)_";
            firstCol = true;
            for (auto& i : iter) {
                comma(firstCol);
                ss << i->getRuntime().count();
            }
            ss << R"_(], "copy_t": [)_";
            firstCol = true;
            for (auto& i : iter) {
                comma(firstCol);
                ss << i->getCopytime().count();
            }
            ss << R"_(], "tuples": [)_";
            firstCol = true;
            for (auto& i : iter) {
                comma(firstCol);
                ss << i->size();
            }
            ss << "]}]";
        }
        ss << "}";

        return ss;
    }

    std::stringstream& genJsonRules(std::stringstream& ss, const std::string& name, size_t maxRows) {
        auto comma = [&ss](bool& first, const std::string& delimiter = ", ") {
            if (!first) {
                ss << delimiter;
            } else {
                first = false;
            }
        };

        ss << '"' << name << R"_(":{)_";

        bool firstRow = true;
        auto rows = ruleTable.getRows();
        std::stable_sort(rows.begin(), rows.end(), [](std::shared_ptr<Row> left, std::shared_ptr<Row> right) {
            return (*left).getDoubleValue(0) > (*right).getDoubleValue(0);
        });
        maxRows = std::min(rows.size(), maxRows);

        for (size_t i = 0; i < maxRows; ++i) {
            Row& row = *rows[i];

            std::vector<std::string> part = Tools::split(row.valueToString(6, 0), ".");
            std::string strRel = "R" + part[0].substr(1);
            Table versionTable = out.getVersions(strRel, row.valueToString(6, 0));

            std::string src;
            if (versionTable.rows.size() > 0) {
                if (versionTable.rows[0]->cells[9] != nullptr) {
                    src = (*versionTable.rows[0]).valueToString(9, 0);
                } else {
                    src = "-";
                }
            } else {
                src = row.valueToString(10, -1);
            }
            comma(firstRow);
            ss << "\n ";

            ss << '"' << row.valueToString(6, 0) << R"_(": [)_";
            ss << '"' << Tools::cleanJsonOut(row.valueToString(5, 0)) << R"_(", )_";
            ss << '"' << Tools::cleanJsonOut(row.valueToString(6, 0)) << R"_(", )_";
            ss << row.getDoubleValue(0) << ", ";
            ss << row.getDoubleValue(1) << ", ";
            ss << row.getDoubleValue(2) << ", ";
            ss << row.getLongValue(4) << ", ";

            ss << '"' << src << R"_(", )_";
            ss << "[";

            bool has_ver = false;
            bool firstCol = true;
            for (auto& _ver_row : versionTable.getRows()) {
                comma(firstCol);
                has_ver = true;
                Row ver_row = *_ver_row;
                ss << '[';
                ss << '"' << Tools::cleanJsonOut(ver_row.valueToString(5, 0)) << R"_(", )_";
                ss << '"' << Tools::cleanJsonOut(ver_row.valueToString(6, 0)) << R"_(", )_";
                ss << ver_row.getDoubleValue(0) << ", ";
                ss << ver_row.getDoubleValue(1) << ", ";
                ss << ver_row.getDoubleValue(2) << ", ";
                ss << ver_row.getLongValue(4) << ", ";
                ss << '"' << src << R"_(", )_";
                ss << ver_row.getLongValue(8);
                ss << ']';
            }

            ss << "], ";

            if (row.valueToString(6, 0).at(0) != 'C') {
                ss << "{}, {}]";
            } else {
                ss << R"_({"tot_t": [)_";

                std::vector<uint64_t> iteration_tuples;
                bool firstCol = true;
                for (auto& i : run->getRelation(row.valueToString(7, 0))->getIterations()) {
                    bool add = false;
                    std::chrono::microseconds totalTime{};
                    uint64_t totalSize = 0L;
                    for (auto& rul : i->getRules()) {
                        if (rul.second->getId() == row.valueToString(6, 0)) {
                            totalTime += rul.second->getRuntime();

                            totalSize += rul.second->size();
                            add = true;
                        }
                    }
                    if (add) {
                        comma(firstCol);
                        ss << totalTime.count();
                        iteration_tuples.push_back(totalSize);
                    }
                }
                ss << R"_(], "tuples": [)_";
                firstCol = true;
                for (auto& i : iteration_tuples) {
                    comma(firstCol);
                    ss << i;
                }

                ss << "]}, {";

                if (has_ver) {
                    ss << R"_("tot_t": [)_";

                    firstCol = true;
                    for (auto& row : versionTable.rows) {
                        comma(firstCol);
                        ss << (*row).getDoubleValue(0);
                    }
                    ss << R"_(], "tuples": [)_";

                    firstCol = true;
                    for (auto& row : versionTable.rows) {
                        comma(firstCol);
                        ss << (*row).getLongValue(4);
                    }
                    ss << ']';
                }
                ss << "}]";
            }
        }
        ss << "\n}";
        return ss;
    }

    std::stringstream& genDlCode(std::stringstream& ss) {
        if (relationTable.getRows().empty()) {
            ss << R"_("code": [Input file not found],\n)_";
            return ss;
        }

        auto comma = [&ss](bool& first, const std::string& delimiter = ", ") {
            if (!first) {
                ss << delimiter;
            } else {
                first = false;
            }
        };

        std::string source_loc = (*relationTable.getRows()[0]).getStringValue(7);
        std::string source_file_loc = Tools::split(source_loc, " ").at(0);
        std::ifstream source_file(source_file_loc);
        if (!source_file.is_open()) {
            ss << R"_("code": [Input file not found],\n)_";
        } else {
            std::string str;
            ss << R"_("code": [)_";
            bool firstCol = true;
            while (getline(source_file, str)) {
                comma(firstCol, ",\n");
                ss << '"' << Tools::cleanJsonOut(str) << '"';
            }
            ss << "],\n";
        }
        return ss;
    }

    std::stringstream& genJsonUsage(std::stringstream& ss) {
        genDlCode(ss);
        auto comma = [&ss](bool& first, const std::string& delimiter = ", ") {
            if (!first) {
                ss << delimiter;
            } else {
                first = false;
            }
        };

        // Add usage statistics
        auto usages = getUsageStats(100);
        auto beginTime = run->getStarttime();

        ss << R"_("usage": [)_";
        bool firstRow = true;
        Usage previousUsage = *usages.begin();
        previousUsage.time = beginTime;
        for (auto usage : usages) {
            comma(firstRow);
            ss << '[';
            ss << (usage.time - beginTime).count() / 1000000.0 << ", ";
            ss << 100.0 * (usage.usertime - previousUsage.usertime) / (usage.time - previousUsage.time)
               << ", ";
            ss << 100.0 * (usage.systemtime - previousUsage.systemtime) / (usage.time - previousUsage.time)
               << ", ";
            ss << usage.maxRSS * 1024 << ", ";
            ss << '"';
            bool firstCol = true;
            for (auto& cur : run->getRelationsAtTime(previousUsage.time, usage.time)) {
                comma(firstCol);
                ss << cur->getName();
            }
            ss << '"';
            ss << ']';
            previousUsage = usage;
        }
        ss << ']';
        return ss;
    }

    std::stringstream& genJsonConfiguration(std::stringstream& ss) {
        auto comma = [&ss](bool& first, const std::string& delimiter = ", ") {
            if (!first) {
                ss << delimiter;
            } else {
                first = false;
            }
        };

        // Add configuration key-value pairs
        ss << R"_("configuration": {)_";
        bool firstRow = true;
        for (auto& kvp :
                ProfileEventSingleton::instance().getDB().getStringMap({"program", "configuration"})) {
            comma(firstRow);
            ss << '"' << kvp.first << R"_(": ")_" << Tools::cleanJsonOut(kvp.second) << '"';
        }
        ss << '}';
        return ss;
    }

    std::stringstream& genJsonAtoms(std::stringstream& ss) {
        auto comma = [&ss](bool& first, const std::string& delimiter = ", ") {
            if (!first) {
                ss << delimiter;
            } else {
                first = false;
            }
        };

        ss << R"_("atoms": {)_";

        bool firstRow = true;
        for (auto& relation : run->getRelationMap()) {
            // Get atoms for non-recursive rules
            for (auto& rule : relation.second->getRuleMap()) {
                comma(firstRow, ", \n");
                ss << '"' << rule.second->getId() << R"_(": [)_";
                bool firstCol = true;
                for (auto& atom : rule.second->getAtoms()) {
                    comma(firstCol);
                    std::string relationName = atom.identifier;
                    relationName = relationName.substr(0, relationName.find('('));
                    auto relation = run->getRelation(relationName);
                    std::string relationSize = relation == nullptr ? "" : std::to_string(relation->size());
                    ss << '[';
                    ss << '"' << Tools::cleanJsonOut(Tools::cleanString(atom.rule)) << R"_(", )_";
                    ss << '"' << Tools::cleanJsonOut(atom.identifier) << R"_(", )_";
                    ss << relationSize << ", ";
                    ss << atom.frequency << ']';
                }
                ss << "]";
            }
            // Get atoms for recursive rules
            for (auto& iteration : relation.second->getIterations()) {
                for (auto& rule : iteration->getRules()) {
                    comma(firstRow, ", \n");
                    ss << '"' << rule.second->getId() << R"_(": [)_";
                    bool firstCol = true;
                    for (auto& atom : rule.second->getAtoms()) {
                        comma(firstCol);
                        std::string relationName = atom.identifier;
                        relationName = relationName.substr(0, relationName.find('('));
                        auto relation = run->getRelation(relationName);
                        std::string relationSize =
                                relation == nullptr ? "" : std::to_string(relation->size());
                        ss << '[';
                        ss << '"' << Tools::cleanJsonOut(Tools::cleanString(atom.rule)) << R"_(", )_";
                        ss << '"' << Tools::cleanJsonOut(atom.identifier) << R"_(", )_";
                        ss << relationSize << ", ";
                        ss << atom.frequency << ']';
                    }
                    ss << "]";
                }
            }
        }

        ss << '}';
        return ss;
    }

    std::string genJson() {
        std::stringstream ss;

        genJsonTop(ss);
        ss << ",\n";
        genJsonRelations(ss, "topRel", 3);
        ss << ",\n";
        genJsonRules(ss, "topRul", 3);
        ss << ",\n";
        genJsonRelations(ss, "rel", relationTable.rows.size());
        ss << ",\n";
        genJsonRules(ss, "rul", ruleTable.rows.size());
        ss << ",\n";
        genJsonUsage(ss);
        ss << ",\n";
        genJsonConfiguration(ss);
        ss << ",\n";
        genJsonAtoms(ss);
        ss << '\n';

        ss << "};\n";

        return ss.str();
    }

};  // namespace profile

}  // namespace profile
}  // namespace souffle
