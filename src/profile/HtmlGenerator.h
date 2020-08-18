/*
 * Souffle - A Datalog Compiler
 * Copyright (c) 2016, The Souffle Developers. All rights reserved
 * Licensed under the Universal Permissive License v 1.0 as shown at:
 * - https://opensource.org/licenses/UPL
 * - <souffle root>/licenses/SOUFFLE-UPL.txt
 */

#pragma once

#include "profile/OutputProcessor.h"
#include "souffle/ProfileEvent.h"
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
    static std::string getHtml(std::string json);

protected:
    static std::string getFirstHalf();
    static std::string getSecondHalf();
    static std::string wrapCss(const std::string& css);
    static std::string wrapJs(const std::string& js);
};

}  // namespace profile
}  // namespace souffle
