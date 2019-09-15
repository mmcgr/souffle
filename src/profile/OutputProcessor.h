/*
 * Souffle - A Datalog Compiler
 * Copyright (c) 2016, The Souffle Developers. All rights reserved
 * Licensed under the Universal Permissive License v 1.0 as shown at:
 * - https://opensource.org/licenses/UPL
 * - <souffle root>/licenses/SOUFFLE-UPL.txt
 */

#pragma once

#include "Cell.h"
#include "CellInterface.h"
#include "Iteration.h"
#include "ProgramRun.h"
#include "Relation.h"
#include "Row.h"
#include "Rule.h"
#include "StringUtils.h"
#include "Table.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace souffle {
namespace profile {

/*
 * Class to format profiler data structures into tables
 */
class OutputProcessor {
private:
    std::shared_ptr<ProgramRun> programRun;

public:
    OutputProcessor() {
        programRun = std::make_shared<ProgramRun>(ProgramRun());
    }

    const std::shared_ptr<ProgramRun>& getProgramRun() const {
        return programRun;
    }

    Table getRelTable() const;

    Table getRulTable() const;

    Table getSubrulTable(std::string strRel, std::string strRul) const;

    Table getAtomTable(std::string strRel, std::string strRul) const;

    Table getVersions(std::string strRel, std::string strRul) const;

    Table getVersionAtoms(std::string strRel, std::string strRul, int version) const;
};

/*
 * rel table :
 * ROW[0] = TOT_T
 * ROW[1] = NREC_T
 * ROW[2] = REC_T
 * ROW[3] = COPY_T
 * ROW[4] = TUPLES
 * ROW[5] = REL NAME
 * ROW[6] = ID
 * ROW[7] = SRC
 * ROW[8] = PERFOR
 * ROW[9] = LOADTIME
 * ROW[10] = SAVETIME
 * ROW[11] = MAXRSSDIFF
 * ROW[12] = READS
 *
 */
Table inline OutputProcessor::getRelTable() const {
    const std::unordered_map<std::string, std::shared_ptr<Relation>>& relationMap =
            programRun->getRelationMap();
    Table table;
    for (auto& rel : relationMap) {
        std::shared_ptr<Relation> r = rel.second;
        Row row(13);
        auto total_time = r->getNonRecTime() + r->getRecTime() + r->getCopyTime();
        row.setCell(0, std::make_shared<Cell<std::chrono::microseconds>>(total_time));
        row.setCell(1, std::make_shared<Cell<std::chrono::microseconds>>(r->getNonRecTime()));
        row.setCell(2, std::make_shared<Cell<std::chrono::microseconds>>(r->getRecTime()));
        row.setCell(3, std::make_shared<Cell<std::chrono::microseconds>>(r->getCopyTime()));
        row.setCell(4, std::make_shared<Cell<long>>(r->size()));
        row.setCell(5, std::make_shared<Cell<std::string>>(r->getName()));
        row.setCell(6, std::make_shared<Cell<std::string>>(r->getId()));
        row.setCell(7, std::make_shared<Cell<std::string>>(r->getLocator()));
        if (total_time.count() != 0) {
            row.setCell(8, std::make_shared<Cell<long>>(r->size() / (total_time.count() / 1000000.0)));
        } else {
            row.setCell(8, std::make_shared<Cell<long>>(r->size()));
        }
        row.setCell(9, std::make_shared<Cell<std::chrono::microseconds>>(r->getLoadtime()));
        row.setCell(10, std::make_shared<Cell<std::chrono::microseconds>>(r->getSavetime()));
        row.setCell(11, std::make_shared<Cell<long>>(r->getMaxRSSDiff()));
        row.setCell(12, std::make_shared<Cell<long>>(r->getReads()));

        table.addRow(std::make_shared<Row>(row));
    }
    return table;
}
/*
 * rul table :
 * ROW[0] = TOT_T
 * ROW[1] = NREC_T
 * ROW[2] = REC_T
 * ROW[3] = COPY_T
 * ROW[4] = TUPLES
 * ROW[5] = RUL NAME
 * ROW[6] = ID
 * ROW[7] = SRC
 * ROW[8] = PERFOR
 * ROW[9] = VER
 * ROW[10]= REL_NAME
 */
Table inline OutputProcessor::getRulTable() const {
    const std::unordered_map<std::string, std::shared_ptr<Relation>>& relationMap =
            programRun->getRelationMap();
    std::unordered_map<std::string, std::shared_ptr<Row>> ruleMap;

    for (auto& rel : relationMap) {
        for (auto& current : rel.second->getRuleMap()) {
            Row row(11);
            std::shared_ptr<Rule> rule = current.second;
            row.setCell(0, std::make_shared<Cell<std::chrono::microseconds>>(rule->getRuntime()));
            row.setCell(1, std::make_shared<Cell<std::chrono::microseconds>>(rule->getRuntime()));
            row.setCell(2, std::make_shared<Cell<std::chrono::microseconds>>(std::chrono::microseconds(0)));
            row.setCell(3, std::make_shared<Cell<std::chrono::microseconds>>(std::chrono::microseconds(0)));
            row.setCell(4, std::make_shared<Cell<long>>(rule->size()));
            row.setCell(5, std::make_shared<Cell<std::string>>(rule->getName()));
            row.setCell(6, std::make_shared<Cell<std::string>>(rule->getId()));
            row.setCell(7, std::make_shared<Cell<std::string>>(rel.second->getName()));
            row.setCell(8, std::make_shared<Cell<long>>(0));
            row.setCell(10, std::make_shared<Cell<std::string>>(rule->getLocator()));
            ruleMap.emplace(rule->getName(), std::make_shared<Row>(row));
        }
        for (auto& iter : rel.second->getIterations()) {
            for (auto& current : iter->getRules()) {
                std::shared_ptr<Rule> rule = current.second;
                if (ruleMap.find(rule->getName()) != ruleMap.end()) {
                    Row row = *ruleMap[rule->getName()];
                    row.setCell(2, std::make_shared<Cell<std::chrono::microseconds>>(
                                           row.getTimeValue(2) + rule->getRuntime()));
                    row.setCell(4, std::make_shared<Cell<long>>(row.getLongValue(4) + rule->size()));
                    row.setCell(0, std::make_shared<Cell<std::chrono::microseconds>>(
                                           row.getTimeValue(0) + rule->getRuntime()));
                    ruleMap[rule->getName()] = std::make_shared<Row>(row);
                } else {
                    Row row(11);
                    row.setCell(0, std::make_shared<Cell<std::chrono::microseconds>>(rule->getRuntime()));
                    row.setCell(1,
                            std::make_shared<Cell<std::chrono::microseconds>>(std::chrono::microseconds(0)));
                    row.setCell(2, std::make_shared<Cell<std::chrono::microseconds>>(rule->getRuntime()));
                    row.setCell(3,
                            std::make_shared<Cell<std::chrono::microseconds>>(std::chrono::microseconds(0)));
                    row.setCell(4, std::make_shared<Cell<long>>(rule->size()));
                    row.setCell(5, std::make_shared<Cell<std::string>>(rule->getName()));
                    row.setCell(6, std::make_shared<Cell<std::string>>(rule->getId()));
                    row.setCell(7, std::make_shared<Cell<std::string>>(rel.second->getName()));
                    row.setCell(8, std::make_shared<Cell<long>>(rule->getVersion()));
                    row.setCell(10, std::make_shared<Cell<std::string>>(rule->getLocator()));
                    ruleMap[rule->getName()] = std::make_shared<Row>(row);
                }
            }
        }
        for (auto& current : ruleMap) {
            std::shared_ptr<Row> row = current.second;
            Row t = *row;
            std::chrono::microseconds val = t.getTimeValue(1) + t.getTimeValue(2) + t.getTimeValue(3);

            t.setCell(0, std::make_shared<Cell<std::chrono::microseconds>>(val));

            if (t.getTimeValue(0).count() != 0) {
                t.setCell(
                        9, std::make_shared<Cell<double>>(t.getLongValue(4) / (t.getDoubleValue(0) * 1000)));
            } else {
                t.setCell(9, std::make_shared<Cell<double>>(t.getLongValue(4) / 1.0));
            }
            current.second = std::make_shared<Row>(t);
        }
    }

    Table table;
    for (auto& current : ruleMap) {
        table.addRow(current.second);
    }
    return table;
}

/*
 * atom table :
 * ROW[0] = clause
 * ROW[1] = atom
 * ROW[2] = level
 * ROW[3] = frequency
 */
Table inline OutputProcessor::getAtomTable(std::string strRel, std::string strRul) const {
    const std::unordered_map<std::string, std::shared_ptr<Relation>>& relationMap =
            programRun->getRelationMap();

    Table table;
    for (auto& current : relationMap) {
        std::shared_ptr<Relation> rel = current.second;

        if (rel->getId() != strRel) {
            continue;
        }

        for (auto& current : rel->getRuleMap()) {
            std::shared_ptr<Rule> rule = current.second;
            if (rule->getId() != strRul) {
                continue;
            }
            for (auto& atom : rule->getAtoms()) {
                Row row(4);
                row.setCell(0, std::make_shared<Cell<std::string>>(atom.rule));
                row.setCell(1, std::make_shared<Cell<std::string>>(atom.identifier));
                row.setCell(2, std::make_shared<Cell<long>>(atom.level));
                row.setCell(3, std::make_shared<Cell<long>>(atom.frequency));

                table.addRow(std::make_shared<Row>(row));
            }
        }
    }
    return table;
}

/*
 * subrule table :
 * ROW[0] = subrule
 */
Table inline OutputProcessor::getSubrulTable(std::string strRel, std::string strRul) const {
    const std::unordered_map<std::string, std::shared_ptr<Relation>>& relationMap =
            programRun->getRelationMap();

    Table table;
    for (auto& current : relationMap) {
        std::shared_ptr<Relation> rel = current.second;

        if (rel->getId() != strRel) {
            continue;
        }

        for (auto& current : rel->getRuleMap()) {
            std::shared_ptr<Rule> rule = current.second;
            if (rule->getId() != strRul) {
                continue;
            }
            for (auto& atom : rule->getAtoms()) {
                Row row(1);
                row.setCell(0, std::make_shared<Cell<std::string>>(atom.rule));

                table.addRow(std::make_shared<Row>(row));
            }
        }
    }
    return table;
}

/*
 * ver table :
 * ROW[0] = TOT_T
 * ROW[1] = NREC_T
 * ROW[2] = REC_T
 * ROW[3] = COPY_T
 * ROW[4] = TUPLES
 * ROW[5] = RUL NAME
 * ROW[6] = ID
 * ROW[7] = SRC
 * ROW[8] = PERFOR
 * ROW[9] = VER
 * ROW[10]= REL_NAME
 */
Table inline OutputProcessor::getVersions(std::string strRel, std::string strRul) const {
    const std::unordered_map<std::string, std::shared_ptr<Relation>>& relationMap =
            programRun->getRelationMap();
    Table table;

    std::shared_ptr<Relation> rel;
    for (auto& current : relationMap) {
        if (current.second->getId() == strRel) {
            rel = current.second;
            break;
        }
    }
    if (rel == nullptr) {
        return table;
    }

    std::unordered_map<std::string, std::shared_ptr<Row>> ruleMap;
    for (auto& iter : rel->getIterations()) {
        for (auto& current : iter->getRules()) {
            std::shared_ptr<Rule> rule = current.second;
            if (rule->getId() == strRul) {
                std::string strTemp =
                        rule->getName() + rule->getLocator() + std::to_string(rule->getVersion());

                if (ruleMap.find(strTemp) != ruleMap.end()) {
                    Row row = *ruleMap[strTemp];
                    row.setCell(2, std::make_shared<Cell<std::chrono::microseconds>>(
                                           row.getTimeValue(2) + rule->getRuntime()));
                    row.setCell(4, std::make_shared<Cell<long>>(row.getLongValue(4) + rule->size()));
                    row.setCell(0, std::make_shared<Cell<std::chrono::microseconds>>(rule->getRuntime()));
                    ruleMap[strTemp] = std::make_shared<Row>(row);
                } else {
                    Row row(10);
                    row.setCell(1,
                            std::make_shared<Cell<std::chrono::microseconds>>(std::chrono::microseconds(0)));
                    row.setCell(2, std::make_shared<Cell<std::chrono::microseconds>>(rule->getRuntime()));
                    row.setCell(3,
                            std::make_shared<Cell<std::chrono::microseconds>>(std::chrono::microseconds(0)));
                    row.setCell(4, std::make_shared<Cell<long>>(rule->size()));
                    row.setCell(5, std::make_shared<Cell<std::string>>(rule->getName()));
                    row.setCell(6, std::make_shared<Cell<std::string>>(rule->getId()));
                    row.setCell(7, std::make_shared<Cell<std::string>>(rel->getName()));
                    row.setCell(8, std::make_shared<Cell<long>>(rule->getVersion()));
                    row.setCell(9, std::make_shared<Cell<std::string>>(rule->getLocator()));
                    row.setCell(0, std::make_shared<Cell<std::chrono::microseconds>>(rule->getRuntime()));
                    ruleMap[strTemp] = std::make_shared<Row>(row);
                }
            }
        }
    }

    for (auto row : ruleMap) {
        Row t = *row.second;
        t.setCell(0, std::make_shared<Cell<std::chrono::microseconds>>(
                             t.getTimeValue(1) + t.getTimeValue(2) + t.getTimeValue(3)));
        ruleMap[row.first] = std::make_shared<Row>(t);
    }

    for (auto& current : ruleMap) {
        table.addRow(current.second);
    }
    return table;
}

/*
 * atom table :
 * ROW[0] = rule
 * ROW[1] = atom
 * ROW[2] = level
 * ROW[3] = frequency
 */
Table inline OutputProcessor::getVersionAtoms(std::string strRel, std::string srcLocator, int version) const {
    const std::unordered_map<std::string, std::shared_ptr<Relation>>& relationMap =
            programRun->getRelationMap();
    Table table;
    std::shared_ptr<Relation> rel;

    for (auto& current : relationMap) {
        if (current.second->getId() == strRel) {
            rel = current.second;
            break;
        }
    }
    if (rel == nullptr) {
        return table;
    }

    for (auto& iter : rel->getIterations()) {
        for (auto& current : iter->getRules()) {
            std::shared_ptr<Rule> rule = current.second;
            if (rule->getLocator() == srcLocator && rule->getVersion() == version) {
                for (auto& atom : rule->getAtoms()) {
                    Row row(4);
                    row.setCell(0, std::make_shared<Cell<std::string>>(atom.rule));
                    row.setCell(1, std::make_shared<Cell<std::string>>(atom.identifier));
                    row.setCell(2, std::make_shared<Cell<long>>(atom.level));
                    row.setCell(3, std::make_shared<Cell<long>>(atom.frequency));
                    table.addRow(std::make_shared<Row>(row));
                }
            }
        }
    }

    return table;
}

}  // namespace profile
}  // namespace souffle
