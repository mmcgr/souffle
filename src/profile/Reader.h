/*
 * Souffle - A Datalog Compiler
 * Copyright (c) 2016, The Souffle Developers. All rights reserved
 * Licensed under the Universal Permissive License v 1.0 as shown at:
 * - https://opensource.org/licenses/UPL
 * - <souffle root>/licenses/SOUFFLE-UPL.txt
 */

#pragma once

#include "profile/Iteration.h"
#include "profile/ProgramRun.h"
#include "profile/Relation.h"
#include "profile/Rule.h"
#include "profile/StringUtils.h"
#include "souffle/ProfileDatabase.h"
#include "souffle/ProfileEvent.h"
#include <cassert>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>
#include <dirent.h>
#include <sys/stat.h>

namespace souffle {
namespace profile {

/*
 * Input reader and processor for log files
 */
class Reader {
private:
    std::string file_loc;
    std::streampos gpos;
    const ProfileDatabase& db = ProfileEventSingleton::instance().getDB();
    bool loaded = false;
    bool online{true};

    std::unordered_map<std::string, std::shared_ptr<Relation>> relationMap{};
    int rel_id{0};

public:
    std::shared_ptr<ProgramRun> run;

    Reader(std::string filename, std::shared_ptr<ProgramRun> run);

    Reader(std::shared_ptr<ProgramRun> run);
    /**
     * Read the contents from file into the class
     */
    void processFile();

    inline bool isLive() {
        return online;
    }

    void addRelation(const DirectoryEntry& relation);

    inline bool isLoaded() {
        return loaded;
    }

    std::string RelationcreateId();

    std::string createId();

protected:
    std::string cleanRelationName(const std::string& relationName);
    std::string extractRelationNameFromAtom(const Atom& atom);
};

}  // namespace profile
}  // namespace souffle
