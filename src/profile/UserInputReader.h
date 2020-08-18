/*
 * Souffle - A Datalog Compiler
 * Copyright (c) 2017, The Souffle Developers. All rights reserved
 * Licensed under the Universal Permissive License v 1.0 as shown at:
 * - https://opensource.org/licenses/UPL
 * - <souffle root>/licenses/SOUFFLE-UPL.txt
 */

#pragma once

#include <string>
#include <vector>

namespace souffle {
namespace profile {

/*
 * A class that reads user input a char at a time allowing for tab completion and history
 */
class InputReader {
public:
    InputReader();
    InputReader(const InputReader& other);
    bool hasReceivedInput();
    void readchar();
    std::string getInput();
    void setPrompt(std::string prompt);
    void appendTabCompletion(std::vector<std::string> commands);
    void appendTabCompletion(std::string command);
    void clearTabCompletion();
    void clearHistory();
    void tabComplete();
    void addHistory(std::string hist);
    void historyUp();
    void historyDown();
    void moveCursor(char direction);
    void moveCursorRight();
    void moveCursorLeft();
    void backspace();
    void clearPrompt(size_t text_len);
    void showFullText(const std::string& text);

private:
    std::string prompt = "Input: ";
    std::vector<std::string> tab_completion;
    std::vector<std::string> history;
    std::string output;
    char current_char = 0;
    size_t cursor_pos = 0;
    size_t hist_pos = 0;
    size_t tab_pos = 0;
    bool in_tab_complete = false;
    bool in_history = false;
    std::string original_hist_val;
    std::string current_hist_val;
    std::string current_tab_val;
    std::string original_tab_val;
    std::vector<std::string> current_tab_completes;
    size_t original_hist_cursor_pos = 0;
    bool inputReceived = false;
};

}  // namespace profile
}  // namespace souffle
