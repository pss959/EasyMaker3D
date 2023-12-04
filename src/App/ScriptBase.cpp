#include "App/ScriptBase.h"

#include <exception>

#include <ion/base/stringutils.h>
#include <ion/math/vectorutils.h>

#include "Util/Read.h"

ScriptBase::ScriptBase() {
#define REG_FUNC_(name, func) \
    RegisterInstrFunc(name, [&](const StrVec &w){ return func(w); });

    REG_FUNC_("key",  ProcessKey_);
    REG_FUNC_("mod",  ProcessMod_);
    REG_FUNC_("stop", ProcessStop_);

#undef REG_FUNC_
}

bool ScriptBase::ReadScript(const FilePath &path) {
    instructions_.clear();

    Str contents;
    if (! Util::ReadFile(path, contents)) {
        std::cerr << "*** Unable to read script file '"
                  << path.ToString() << "'\n";
        return false;
    }

    file_path_   = path;
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

void ScriptBase::RegisterInstrFunc(const Str &name, const InstrFunc &func) {
    if (func_map_.find(name) != func_map_.end())
        Error("Duplicate functions for '" + name + "' instruction");
    else
        func_map_[name] = func;
}

void ScriptBase::Error(const Str &message) {
    std::cerr << file_path_.ToString() << ":" << line_number_
              << ": *** " << message << "'\n";
}

bool ScriptBase::ParseVector3f(const StrVec &words, size_t index, Vector3f &v) {
    return (ParseFloat(words[index + 0], v[0]) &&
            ParseFloat(words[index + 1], v[1]) &&
            ParseFloat(words[index + 2], v[2]));
}

bool ScriptBase::ParseFloat(const Str &s, float &f) {
    try {
        size_t chars_processed;
        f = std::stof(s, &chars_processed);

        // Make sure there are no extra characters at the end.
        return chars_processed == s.size();
    }
    catch (std::exception &) {
        return false;
    }
}

bool ScriptBase::ParseFloat01(const Str &s, float &f) {
    // Make sure the number is in range.
    return ParseFloat(s, f) && f >= 0.f && f <= 1.f;
}

bool ScriptBase::ParseN(const Str &s, size_t &n) {
    try {
        size_t chars_processed;
        n = std::stoll(s, &chars_processed, 10);
        return chars_processed == s.size();  // No extra characters at the end.
    }
    catch (std::exception &) {
        return false;
    }
}

bool ScriptBase::ProcessLine_(const Str &line) {
    // Skip empty lines and comments.
    if (line.empty() || line[0] == '#')
        return true;

    const auto words = GetWords_(line);

    // Verify that the instruction has a corresponding function.
    const auto it = func_map_.find(words[0]);
    if (it == func_map_.end()) {
        Error("Unknown instruction type '" + words[0] + "'");
        return false;
    }

    // Invoke the function.
    if (auto instr = it->second(words)) {
        instr->name        = words[0];
        instr->line_number = line_number_;
        instructions_.push_back(instr);
        return true;
    }
    return false;
}

StrVec ScriptBase::GetWords_(const Str &line) {
    StrVec words = ion::base::SplitString(line, " \t");

    // Check for comments.
    for (size_t i = 0; i < words.size(); ++i) {
        if (words[i].starts_with("#")) {
            words.resize(i);
            break;
        }
    }
    return words;
}

ScriptBase::InstrPtr ScriptBase::ProcessKey_(const StrVec &words) {
    KeyInstrPtr kinst;
    if (words.size() != 2U) {
        Error("Bad syntax for key instruction");
    }
    else {
        // words[1] is the key string
        kinst.reset(new KeyInstr);
        Str error;
        if (! Event::ParseKeyString(words[1], kinst->modifiers,
                                    kinst->key_name, error)) {
            Error(error + " in key instruction");
            kinst.reset();
        }
    }
    return kinst;
}

ScriptBase::InstrPtr ScriptBase::ProcessMod_(const StrVec &words) {
    ModInstrPtr minst;
    if (words.size() != 2U || (words[1] != "on" && words[1] != "off")) {
        Error("Bad syntax for mod instruction");
    }
    else {
        minst.reset(new ModInstr);
        minst->is_on = words[1] == "on";
    }
    return minst;
}

ScriptBase::InstrPtr ScriptBase::ProcessStop_(const StrVec &words) {
    StopInstrPtr sinst;
    if (words.size() != 1U) {
        Error("Bad syntax for stop instruction");
    }
    else {
        sinst.reset(new StopInstr);
    }
    return sinst;
}
