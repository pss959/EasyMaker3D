#include "App/SnapScript.h"

#include <ion/base/stringutils.h>

#include "Util/FilePath.h"
#include "Util/Read.h"

bool SnapScript::ReadScript(const std::string &file_name) {
    instructions_.clear();

    std::string contents;
    if (! Util::ReadFile(FilePath(file_name), contents)) {
        std::cerr << "*** Unable to read script file '" << file_name << "'\n";
        return false;
    }

    file_name_   = file_name;
    line_number_ = 1;

    // Split the input into lines and process each one.
    const auto lines = ion::base::SplitString(contents, "\n");
    size_t line_number = 1;
    for (const auto &line: lines) {
        if (! ProcessLine_(ion::base::TrimStartAndEndWhitespace(line)))
            return false;
        ++line_number;
    }
    return true;
}

bool SnapScript::ProcessLine_(const std::string &line) {
    if (line[0] == '#')  // Comment
        return true;

    const auto words = ion::base::SplitString(line, " \t");
    Instruction instr;
    instr.type = words[0];

    if (instr.type      == "load") {
    }
    else if (instr.type == "snap") {
    }
    else if (instr.type == "undo") {
    }
    else if (instr.type == "redo") {
    }
    else {
        return Error_("Unknown instruction type '" + instr.type + "'");
    }
    instructions_.push_back(instr);
    return true;
}

bool SnapScript::Error_(const std::string &message) {
    std::cerr << "*** " << message << " on line " << line_number_
              << " in '" << file_name_ << "'\n";
    return false;
}
