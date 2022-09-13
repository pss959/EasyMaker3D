#include "App/SnapScript.h"

#include <ion/base/stringutils.h>
#include <ion/math/vectorutils.h>

#include "Util/Enum.h"
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

    bool ok;
    if      (instr.type == "action") ok = ProcessAction_(words,  instr);
    else if (instr.type == "hand")   ok = ProcessHand_(words,    instr);
    else if (instr.type == "load")   ok = ProcessLoad_(words,    instr);
    else if (instr.type == "redo")   ok = ProcessRedo_(words,    instr);
    else if (instr.type == "select") ok = ProcessSelect_(words,  instr);
    else if (instr.type == "snap")   ok = ProcessSnap_(words,    instr);
    else if (instr.type == "stage")  ok = ProcessStage_(words,   instr);
    else if (instr.type == "touch")  ok = ProcessTouch_(words,   instr);
    else if (instr.type == "undo")   ok = ProcessUndo_(words,    instr);
    else if (instr.type == "view")   ok = ProcessView_(words,    instr);
    else ok = Error_("Unknown instruction type '" + instr.type + "'");
    if (! ok)
        return false;

    instructions_.push_back(instr);
    return true;
}

bool SnapScript::ProcessAction_(const Words_ &words, Instruction &instr) {
    if (words.size() != 2U)
        return Error_("Bad syntax for action instruction");

    if (! Util::EnumFromString(words[1], instr.action))
        return Error_("Unknown action name for action instruction");

    return true;
}

bool SnapScript::ProcessHand_(const Words_ &words, Instruction &instr) {
    if (words.size() != 9U)
        return Error_("Bad syntax for hand instruction");

    if (words[1] == "L")
        instr.hand = Hand::kLeft;
    else if (words[1] == "R")
        instr.hand = Hand::kRight;
    else
        return Error_("Invalid hand (L/R) for hand instruction");

    if (words[2] == "Oculus_Touch" || words[2] == "Vive" || words[2] == "None")
        instr.hand_type = words[2];
    else
        return Error_("Invalid controller type for hand instruction");

    Vector3f v;
    if (! ParseVector3f_(words, 3, v))
        return Error_("Invalid position floats for hand instruction");
    instr.hand_pos = Point3f(v);

    if (! ParseVector3f_(words, 6, instr.hand_dir))
        return Error_("Invalid direction floats for hand instruction");

    return true;
}

bool SnapScript::ProcessLoad_(const Words_ &words, Instruction &instr) {
    if (words.size() != 2U)
        return Error_("Bad syntax for load instruction");

    instr.file_name = words[1];
    return true;
}

bool SnapScript::ProcessRedo_(const Words_ &words, Instruction &instr) {
    if (words.size() != 2U)
        return Error_("Bad syntax for redo instruction");

    if (! ParseN_(words[1], instr.count))
        return Error_("Invalid count for redo instruction");

    return true;
}

bool SnapScript::ProcessSelect_(const Words_ &words, Instruction &instr) {
    // No names is a valid selection (deselects all).
    instr.names.insert(instr.names.begin(), words.begin() + 1, words.end());
    return true;
}

bool SnapScript::ProcessSnap_(const Words_ &words, Instruction &instr) {
    if (words.size() != 6U)
        return Error_("Bad syntax for snap instruction");

    float x, y, w, h;
    if (! ParseFloat01_(words[1], x) || ! ParseFloat01_(words[2], y) ||
        ! ParseFloat01_(words[3], w) || ! ParseFloat01_(words[4], h))
        return Error_("Invalid rectangle floats for snap instruction");

    if (x + w > 1.f || y + h > 1.f)
        return Error_("Rectangle is out of bounds for snap instruction");

    instr.rect.SetWithSize(Point2f(x, y), Vector2f(w, h));
    instr.file_name = words[5];
    return true;
}

bool SnapScript::ProcessStage_(const Words_ &words, Instruction &instr) {
    if (words.size() != 3U)
        return Error_("Bad syntax for stage instruction");

    float scale, angle;
    if (! ParseFloat_(words[1], scale) || ! ParseFloat_(words[2], angle))
        return Error_("Invalid scale/rotation data for stage instruction");

    instr.stage_scale = scale;
    instr.stage_angle = Anglef::FromDegrees(angle);
    return true;
}

bool SnapScript::ProcessTouch_(const Words_ &words, Instruction &instr) {
    if (words.size() != 2U || (words[1] != "on" && words[1] != "off"))
        return Error_("Bad syntax for touch instruction");

    instr.touch_on = words[1] == "on";
    return true;
}

bool SnapScript::ProcessUndo_(const Words_ &words, Instruction &instr) {
    if (words.size() != 2U)
        return Error_("Bad syntax for undo instruction");

    if (! ParseN_(words[1], instr.count))
        return Error_("Invalid count for undo instruction");

    return true;
}

bool SnapScript::ProcessView_(const Words_ &words, Instruction &instr) {
    if (words.size() != 4U)
        return Error_("Bad syntax for view instruction");

    if (! ParseVector3f_(words, 1, instr.view_dir))
        return Error_("Invalid direction floats for view instruction");

    ion::math::Normalize(&instr.view_dir);
    return true;
}

bool SnapScript::Error_(const std::string &message) {
    std::cerr << "*** " << message << " on line " << line_number_
              << " in '" << file_name_ << "'\n";
    return false;
}

bool SnapScript::ParseVector3f_(const Words_ &words, size_t index,
                                Vector3f &v) {
    return (ParseFloat_(words[index + 0], v[0]) &&
            ParseFloat_(words[index + 1], v[1]) &&
            ParseFloat_(words[index + 2], v[2]));
}

bool SnapScript::ParseFloat_(const std::string &s, float &f) {
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

bool SnapScript::ParseFloat01_(const std::string &s, float &f) {
    // Make sure the number is in range.
    return ParseFloat_(s, f) && f >= 0.f && f <= 1.f;
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
