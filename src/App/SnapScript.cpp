#include "App/SnapScript.h"

#include <ion/base/stringutils.h>
#include <ion/math/vectorutils.h>

#include "Util/Enum.h"
#include "Util/Read.h"

bool SnapScript::ReadScript(const FilePath &path) {
    instructions_.clear();

    std::string contents;
    if (! Util::ReadFile(path, contents)) {
        std::cerr << "*** Unable to read script file '"
                  << path.ToString() << "'\n";
        if (! path.Exists())
            std::cerr << "XXXX path does not exist!!!\n";
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

bool SnapScript::ProcessLine_(const std::string &line) {
    // Skip empty lines and comments.
    if (line.empty() || line[0] == '#')
        return true;

    const auto words = ion::base::SplitString(line, " \t");
    Instr::Type type;
    if (! GetInstructionType_(words[0], type))
        return Error_("Unknown instruction type '" + words[0] + "'");

    InstrPtr instr;
    switch (type) {
      case Instr::Type::kAction:   instr = ProcessAction_(words);   break;
      case Instr::Type::kDrag:     instr = ProcessDrag_(words);     break;
      case Instr::Type::kHand:     instr = ProcessHand_(words);     break;
      case Instr::Type::kKey:      instr = ProcessKey_(words);     break;
      case Instr::Type::kLoad:     instr = ProcessLoad_(words);     break;
      case Instr::Type::kSelect:   instr = ProcessSelect_(words);   break;
      case Instr::Type::kSettings: instr = ProcessSettings_(words); break;
      case Instr::Type::kSnap:     instr = ProcessSnap_(words);     break;
      case Instr::Type::kStage:    instr = ProcessStage_(words);    break;
      case Instr::Type::kTouch:    instr = ProcessTouch_(words);    break;
      case Instr::Type::kView:     instr = ProcessView_(words);     break;
    }
    if (! instr)
        return false;

    instr->type = type;
    instructions_.push_back(instr);
    return true;
}

bool SnapScript::GetInstructionType_(const std::string &word,
                                     Instr::Type &type) {
    for (auto t: Util::EnumValues<Instr::Type>()) {
        if (Util::ToLowerCase(Util::EnumToWord(t)) == word) {
            type = t;
            return true;
        }
    }
    return false;
}

SnapScript::InstrPtr SnapScript::ProcessAction_(const Words &words) {
    ActionInstrPtr ainst;
    Action         action;
    if (words.size() != 2U) {
        Error_("Bad syntax for action instruction");
    }
    else if (! Util::EnumFromString(words[1], action)) {
        Error_("Unknown action name for action instruction");
    }
    else {
        ainst.reset(new ActionInstr);
        ainst->action = action;
    }
    return ainst;
}

SnapScript::InstrPtr SnapScript::ProcessDrag_(const Words &words) {
    using DIPhase = DragInstr::Phase;

    DragInstrPtr dinst;
    float x, y;
    if (words.size() != 4U) {
        Error_("Bad syntax for drag instruction");
    }
    else if (words[1] != "start" &&
             words[1] != "continue" &&
             words[1] != "end") {
        Error_("Invalid phase (start/continue/end) for drag instruction");
    }
    else if (! ParseFloat01_(words[2], x) || ! ParseFloat01_(words[3], y)) {
        Error_("Invalid x or y floats for drag instruction");
    }
    else {
        dinst.reset(new DragInstr);
        dinst->phase = words[1] == "start" ? DIPhase::kStart :
            words[1] == "continue" ? DIPhase::kContinue : DIPhase::kEnd;
        dinst->pos.Set(x, y);
    }
    return dinst;
}

SnapScript::InstrPtr SnapScript::ProcessHand_(const Words &words) {
    HandInstrPtr hinst;
    Vector3f     pos, dir;
    if (words.size() != 9U) {
         Error_("Bad syntax for hand instruction");
    }
    else if (words[1] != "L" && words[1] != "R") {
        Error_("Invalid hand (L/R) for hand instruction");
    }
    else if (words[2] != "Oculus_Touch" &&
             words[2] != "Vive" &&
             words[2] != "None") {
        Error_("Invalid controller type for hand instruction");
    }
    else if (! ParseVector3f_(words, 3, pos)) {
        Error_("Invalid position floats for hand instruction");
    }
    else if (! ParseVector3f_(words, 6, dir)) {
        Error_("Invalid direction floats for hand instruction");
    }
    else {
        hinst.reset(new HandInstr);
        hinst->hand = words[1] == "L" ? Hand::kLeft : Hand::kRight;
        hinst->controller = words[2];
        hinst->pos = Point3f(pos);
        hinst->dir = dir;
    }
    return hinst;
}

SnapScript::InstrPtr SnapScript::ProcessKey_(const Words &words) {
    KeyInstrPtr kinst;
    if (words.size() < 2U) {
        Error_("Bad syntax for key instruction");
    }
    else {
        // words[1] is the key name. All the rest are modifiers.
        bool ok = true;
        for (size_t i = 2U; i < words.size(); ++i) {
            if (words[i] != "ctrl" &&
                words[i] != "alt") {
                Error_("Bad modifier for key instruction");
                ok = false;
            }
        }
        if (ok) {
            kinst.reset(new KeyInstr);
            kinst->key = words[1];
            kinst->is_ctrl_on = kinst->is_alt_on = false;
            for (size_t i = 2U; i < words.size(); ++i) {
                if (words[i] == "ctrl")
                    kinst->is_ctrl_on = true;
                else
                    kinst->is_alt_on = true;
            }
        }
    }
    return kinst;
}

SnapScript::InstrPtr SnapScript::ProcessLoad_(const Words &words) {
    LoadInstrPtr linst;
    if (words.size() != 2U) {
        Error_("Bad syntax for load instruction");
    }
    else {
        linst.reset(new LoadInstr);
        linst->file_name = words[1];
    }
    return linst;
}

SnapScript::InstrPtr SnapScript::ProcessSelect_(const Words &words) {
    // No names is a valid selection (deselects all).
    SelectInstrPtr sinst(new SelectInstr);
    sinst->names.insert(sinst->names.begin(), words.begin() + 1, words.end());
    return sinst;
}

SnapScript::InstrPtr SnapScript::ProcessSettings_(const Words &words) {
    SettingsInstrPtr sinst;
    if (words.size() != 2U) {
        Error_("Bad syntax for settings instruction");
    }
    else {
        sinst.reset(new SettingsInstr);
        sinst->file_name = words[1];
    }
    return sinst;
}

SnapScript::InstrPtr SnapScript::ProcessSnap_(const Words &words) {
    SnapInstrPtr sinst;
    float x, y, w, h;
    if (words.size() != 6U) {
        Error_("Bad syntax for snap instruction");
    }
    else if (! ParseFloat01_(words[1], x) || ! ParseFloat01_(words[2], y) ||
             ! ParseFloat01_(words[3], w) || ! ParseFloat01_(words[4], h)) {
        Error_("Invalid rectangle floats for snap instruction");
    }
    else if (x + w > 1.f || y + h > 1.f) {
        Error_("Rectangle is out of bounds for snap instruction");
    }
    else {
        sinst.reset(new SnapInstr);
        sinst->rect.SetWithSize(Point2f(x, y), Vector2f(w, h));
        sinst->file_name = words[5];
    }
    return sinst;
}

SnapScript::InstrPtr SnapScript::ProcessStage_(const Words &words) {
    StageInstrPtr sinst;
    float scale, angle;
    if (words.size() != 3U) {
        Error_("Bad syntax for stage instruction");
    }
    else if (! ParseFloat_(words[1], scale) || ! ParseFloat_(words[2], angle)) {
        Error_("Invalid scale/rotation data for stage instruction");
    }
    else {
        sinst.reset(new StageInstr);
        sinst->scale = scale;
        sinst->angle = Anglef::FromDegrees(angle);
    }
    return sinst;
}

SnapScript::InstrPtr SnapScript::ProcessTouch_(const Words &words) {
    TouchInstrPtr tinst;
    if (words.size() != 2U || (words[1] != "on" && words[1] != "off")) {
        Error_("Bad syntax for touch instruction");
    }
    else {
        tinst.reset(new TouchInstr);
        tinst->is_on = words[1] == "on";
    }
    return tinst;
}

SnapScript::InstrPtr SnapScript::ProcessView_(const Words &words) {
    ViewInstrPtr vinst;
    Vector3f     dir;
    if (words.size() != 4U) {
        Error_("Bad syntax for view instruction");
    }
    else if (! ParseVector3f_(words, 1, dir)) {
        Error_("Invalid direction floats for view instruction");
    }
    else {
        ion::math::Normalize(&dir);
        vinst.reset(new ViewInstr);
        vinst->dir = dir;
    }
    return vinst;
}

bool SnapScript::Error_(const std::string &message) {
    std::cerr << "*** " << message << " on line " << line_number_
              << " in '" << file_path_.ToString() << "'\n";
    return false;
}

bool SnapScript::ParseVector3f_(const Words &words, size_t index,
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
