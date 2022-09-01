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
    const auto lines = ion::base::SplitStringWithoutSkipping(contents, "\n");
    for (const auto &line: lines) {
        if (! ProcessLine_(ion::base::TrimStartAndEndWhitespace(line)))
            return false;
        ++line_number_;
    }
    return true;
}

bool SnapScript::ProcessLine_(const std::string &line) {
    // Skip empty lines and comments.
    if (line.empty() || line[0] == '#')
        return true;

    const auto words = ion::base::SplitString(line, " \t");
    Instruction instr;
    instr.type = words[0];

    if (instr.type      == "load") {
        if (words.size() != 2U)
            return Error_("Bad syntax for load instruction");
        instr.file_name = words[1];
    }
    else if (instr.type == "snap") {
        if (words.size() != 6U)
            return Error_("Bad syntax for snap instruction");
        float x, y, w, h;
        if (! ParseFloat01_(words[1], x) || ! ParseFloat01_(words[2], y) ||
            ! ParseFloat01_(words[3], w) || ! ParseFloat01_(words[4], h))
            return Error_("Invalid rectangle floats for snap instruction");
        else if (x + w > 1.f || y + h > 1.f)
            return Error_("Rectangle is out of bounds for snap instruction");
        instr.rect.SetWithSize(Point2f(x, y), Vector2f(w, h));
        instr.file_name = words[5];
    }
    else if (instr.type == "undo") {
        if (! ParseN_(words[1], instr.count))
            return Error_("Invalid count for undo instruction");
    }
    else if (instr.type == "redo") {
        if (! ParseN_(words[1], instr.count))
            return Error_("Invalid count for redo instruction");
    }
    else if (instr.type == "select") {
        instr.names.insert(instr.names.begin(), words.begin() + 1, words.end());
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

bool SnapScript::ParseFloat01_(const std::string &s, float &f) {
    try {
        size_t chars_processed;
        f = std::stof(s, &chars_processed);

        // Make sure there are no extra characters at the end and the number is
        // in range.
        return chars_processed == s.size() && f >= 0.f && f <= 1.f;
    }
    catch (std::exception &) {
        return false;
    }
}

bool SnapScript::ParseN_(const std::string &s, size_t &n) {
    try {
        size_t chars_processed;
        n = std::stoll(s, &chars_processed, 10);
        return chars_processed == s.size();  // No extra characters at the end.
    }
    catch (std::exception &) {
        return false;
    }
}
