#include "App/Script.h"

#include <exception>

#include <ion/base/stringutils.h>
#include <ion/math/vectorutils.h>

#include "Util/Enum.h"
#include "Util/General.h"
#include "Util/Read.h"

// ----------------------------------------------------------------------------
// Helper functions.
// ----------------------------------------------------------------------------

namespace {

static Rotationf ComputeHandRotation_(Hand hand, const Vector3f &laser_dir,
                                      const Vector3f &guide_dir) {
    const Rotationf laser_rot =
        Rotationf::RotateInto(-Vector3f::AxisZ(), laser_dir);
    Rotationf guide_rot;
    if (guide_dir != Vector3f::Zero()) {
        const Vector3f guide_start = laser_rot *
            (hand == Hand::kLeft ? Vector3f::AxisX() : -Vector3f::AxisX());
        guide_rot = Rotationf::RotateInto(guide_start, guide_dir);
    }
    return guide_rot * laser_rot;
}

}  // anonymous namespace

// ----------------------------------------------------------------------------
// Script class functions.
// ----------------------------------------------------------------------------

Script::Script() {
    auto reg_func = [&](const Str &name, const InstrFunc_ &func){
        if (func_map_.find(name) != func_map_.end())
            Error_("Duplicate functions for '" + name + "' instruction");
        else
            func_map_[name] = func;
    };

    // E.g, maps REG_FUNC_(Action) to ProcessAction_(cast to ActionInstr).
#define REG_FUNC_(name)                                                 \
    reg_func(Util::ToLowerCase(#name),                                  \
             [&](const StrVec &w){ return Parse ## name ##_(w); })

    REG_FUNC_(Action);
    REG_FUNC_(Caption);
    REG_FUNC_(Click);
    REG_FUNC_(Drag);
    REG_FUNC_(Focus);
    REG_FUNC_(Hand);
    REG_FUNC_(HandPos);
    REG_FUNC_(Highlight);
    REG_FUNC_(Key);
    REG_FUNC_(Load);
    REG_FUNC_(MoveOver);
    REG_FUNC_(MoveTo);
    REG_FUNC_(Section);
    REG_FUNC_(Select);
    REG_FUNC_(Settings);
    REG_FUNC_(Snap);
    REG_FUNC_(SnapObj);
    REG_FUNC_(Stage);
    REG_FUNC_(State);
    REG_FUNC_(Stop);
    REG_FUNC_(View);
    REG_FUNC_(Wait);

#undef REG_FUNC_
}

bool Script::ReadScript(const FilePath &path) {
    instructions_.clear();

    Str contents;
    if (! Util::ReadFile(path, contents)) {
        std::cerr << "*** Unable to read script file '"
                  << path.ToString() << "'\n";
        return false;
    }

    file_path_   = path;
    line_number_ = 1;

    // Split the input into lines and parse each one.
    const auto lines = ion::base::SplitStringWithoutSkipping(contents, "\n");
    for (const auto &line: lines) {
        if (! ParseLine_(ion::base::TrimStartAndEndWhitespace(line)))
            return false;
        ++line_number_;
    }
    return true;
}

void Script::Error_(const Str &message) {
    std::cerr << file_path_.ToString() << ":" << line_number_
              << ": *** " << message << "'\n";
}

// ----------------------------------------------------------------------------
// Line parsing.
// ----------------------------------------------------------------------------

bool Script::ParseLine_(const Str &line) {
    // Skip empty lines and comments.
    if (line.empty() || line[0] == '#')
        return true;

    StrVec words = ion::base::SplitString(line, " \t");

    // Check for comments.
    for (size_t i = 0; i < words.size(); ++i) {
        if (words[i].starts_with("#")) {
            words.resize(i);
            break;
        }
    }

    // Verify that the instruction has a corresponding function.
    const auto it = func_map_.find(words[0]);
    if (it == func_map_.end()) {
        Error_("Unknown instruction type '" + words[0] + "'");
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

bool Script::ParseVector3f_(const StrVec &words, size_t index, Vector3f &v) {
    return (ParseFloat_(words[index + 0], v[0]) &&
            ParseFloat_(words[index + 1], v[1]) &&
            ParseFloat_(words[index + 2], v[2]));
}

bool Script::ParseFloat_(const Str &s, float &f) {
    try {
        size_t chars_parsed;
        f = std::stof(s, &chars_parsed);

        // Make sure there are no extra characters at the end.
        return chars_parsed == s.size();
    }
    catch (std::exception &) {
        return false;
    }
}

bool Script::ParseFloat01_(const Str &s, float &f) {
    // Make sure the number is in range.
    return ParseFloat_(s, f) && f >= 0.f && f <= 1.f;
}

bool Script::ParseN_(const Str &s, size_t &n) {
    try {
        size_t chars_parsed;
        n = std::stoll(s, &chars_parsed, 10);
        return chars_parsed == s.size();  // No extra characters at the end.
    }
    catch (std::exception &) {
        return false;
    }
}

// ----------------------------------------------------------------------------
// Parsing specific instructions.
// ----------------------------------------------------------------------------

Script::InstrPtr Script::ParseAction_(const StrVec &words) {
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

Script::InstrPtr Script::ParseCaption_(const StrVec &words) {
    CaptionInstrPtr cinst;
    float           x, y, duration;
    if (words.size() < 5U) {
        Error_("Bad syntax for caption instruction");
    }
    else if (! ParseFloat01_(words[1], x) || ! ParseFloat01_(words[2], y) ||
             ! ParseFloat_(words[3], duration)) {
        Error_("Invalid x, y, or duration floats for caption instruction");
    }
    else {
        cinst.reset(new CaptionInstr);
        cinst->pos.Set(x, y);
        cinst->duration = duration;
        cinst->text =
            Util::ReplaceString(
                Util::JoinStrings(StrVec(words.begin() + 4, words.end())),
                ";", "\n");
    }
    return cinst;
}

Script::InstrPtr Script::ParseClick_(const StrVec &words) {
    ClickInstrPtr cinst;
    if (words.size() != 1U) {
        Error_("Bad syntax for click instruction");
    }
    else {
        cinst.reset(new ClickInstr);
    }
    return cinst;
}

Script::InstrPtr Script::ParseDrag_(const StrVec &words) {
    DragInstrPtr dinst;
    float        dx, dy, duration;
    if (words.size() < 4 || words.size() > 5) {
        Error_("Bad syntax for drag instruction");
    }
    else if (! ParseFloat_(words[1], dx) || ! ParseFloat_(words[2], dy) ||
             ! ParseFloat_(words[3], duration)) {
        Error_("Invalid dx, dy, or duration floats for drag instruction");
    }
    else if (duration <= 0) {
        Error_("Duration for drag instruction must be positive");
    }
    else if (words.size() == 5 &&
             (words[4] != "L" && words[4] != "M" && words[4] != "R")) {
        Error_("Invalid mouse button for drag instruction");
    }
    else {
        dinst.reset(new DragInstr);
        dinst->motion.Set(dx, dy);
        dinst->duration = duration;
        dinst->button = words.size() == 5 ? words[4] : "L";
    }
    return dinst;
}

Script::InstrPtr Script::ParseFocus_(const StrVec &words) {
    FocusInstrPtr finst;
    if (words.size() != 2U) {
        Error_("Bad syntax for focus instruction");
    }
    else {
        finst.reset(new FocusInstr);
        finst->pane_name = words[1];
    }
    return finst;
}

Script::InstrPtr Script::ParseHand_(const StrVec &words) {
    HandInstrPtr hinst;
    Vector3f     pos, dir;
    if (words.size() != 3U) {
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
    else {
        hinst.reset(new HandInstr);
        hinst->hand = words[1] == "L" ? Hand::kLeft : Hand::kRight;
        hinst->controller = words[2];
    }
    return hinst;
}

Script::InstrPtr Script::ParseHandPos_(const StrVec &words) {
    HandPosInstrPtr hinst;
    Vector3f        pos;
    Vector3f        laser_dir;
    Vector3f        guide_dir;
    if (words.size() != 11U) {
         Error_("Bad syntax for handpos instruction");
    }
    else if (words[1] != "L" && words[1] != "R") {
        Error_("Invalid hand (L/R) for handpos instruction");
    }
    else if (! ParseVector3f_(words, 2, pos)) {
        Error_("Invalid position floats for handpos instruction");
    }
    else if (! ParseVector3f_(words, 5, laser_dir)) {
        Error_("Invalid laser direction floats for handpos instruction");
    }
    else if (! ParseVector3f_(words, 8, guide_dir)) {
        Error_("Invalid guide direction floats for handpos instruction");
    }
    else {
        hinst.reset(new HandPosInstr);
        hinst->hand = words[1] == "L" ? Hand::kLeft : Hand::kRight;
        hinst->pos = Point3f(pos);
        hinst->rot = ComputeHandRotation_(hinst->hand, laser_dir, guide_dir);
    }
    return hinst;
}

Script::InstrPtr Script::ParseHighlight_(
    const StrVec &words) {
    HighlightInstrPtr hinst;
    float duration, margin = 0;
    if (words.size() < 3U || words.size() > 4U) {
        Error_("Bad syntax for highlight instruction");
    }
    else if (! ParseFloat_(words[2], duration)) {
        Error_("Invalid duration float for highlight instruction");
    }
    else if (words.size() == 4U && ! ParseFloat_(words[3], margin)) {
        Error_("Invalid margin float for highlight instruction");
    }
    else if (duration <= 0) {
        Error_("Duration for highlight instruction must be positive");
    }
    else {
        hinst.reset(new HighlightInstr);
        hinst->path_string = words[1];
        hinst->margin      = margin;
        hinst->duration     = duration;
    }
    return hinst;
}

Script::InstrPtr Script::ParseKey_(const StrVec &words) {
    KeyInstrPtr kinst;
    if (words.size() != 2U) {
        Error_("Bad syntax for key instruction");
    }
    else {
        // words[1] is the key string
        kinst.reset(new KeyInstr);
        kinst->key_string = words[1];
        Event::Modifiers modifiers;
        Str              key_name;
        Str              error;
        if (! Event::ParseKeyString(words[1], modifiers, key_name, error)) {
            Error_(error + " in key instruction");
            kinst.reset();
        }
    }
    return kinst;
}

Script::InstrPtr Script::ParseLoad_(const StrVec &words) {
    LoadInstrPtr linst;
    if (words.size() > 2U) {
        Error_("Bad syntax for load instruction");
    }
    else {
        linst.reset(new LoadInstr);
        if (words.size() == 2U)
            linst->file_name = words[1];
    }
    return linst;
}

Script::InstrPtr Script::ParseMoveOver_(const StrVec &words) {
    MoveOverInstrPtr minst;
    float            duration;
    if (words.size() != 3U) {
        Error_("Bad syntax for moveover instruction");
    }
    else if (! ParseFloat_(words[2], duration)) {
        Error_("Invalid duration float for moveover instruction");
    }
    else {
        minst.reset(new MoveOverInstr);
        minst->path_string = words[1];
        minst->duration     = duration;
    }
    return minst;
}

Script::InstrPtr Script::ParseMoveTo_(const StrVec &words) {
    MoveToInstrPtr minst;
    float          x, y, duration;
    if (words.size() != 4) {
        Error_("Bad syntax for moveto instruction");
    }
    else if (! ParseFloat01_(words[1], x) || ! ParseFloat01_(words[2], y) ||
             ! ParseFloat_(words[3], duration)) {
        Error_("Invalid x, y, or duration floats for moveto instruction");
    }
    else {
        minst.reset(new MoveToInstr);
        minst->pos.Set(x, y);
        minst->duration = duration;
    }
    return minst;
}

Script::InstrPtr Script::ParseSection_(const StrVec &words) {
    SectionInstrPtr sinst;
    if (words.size() < 3U) {
        Error_("Bad syntax for section instruction");
    }
    else {
        sinst.reset(new SectionInstr);
        sinst->tag = words[1];
        sinst->title =
            Util::JoinStrings(StrVec(words.begin() + 2, words.end()));
    }
    return sinst;
}

Script::InstrPtr Script::ParseSelect_(const StrVec &words) {
    // No names is a valid selection (deselects all).
    SelectInstrPtr sinst(new SelectInstr);
    sinst->names.insert(sinst->names.begin(), words.begin() + 1, words.end());
    return sinst;
}

Script::InstrPtr Script::ParseSettings_(const StrVec &words) {
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

Script::InstrPtr Script::ParseSnap_(const StrVec &words) {
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

Script::InstrPtr Script::ParseSnapObj_(const StrVec &words) {
    SnapObjInstrPtr sinst;
    float margin = 0;
    if (words.size() < 3U || words.size() > 4U) {
        Error_("Bad syntax for snapobj instruction");
    }
    else if (words.size() == 4U && ! ParseFloat_(words[3], margin)) {
        Error_("Invalid margin float for snapobj instruction");
    }
    else {
        sinst.reset(new SnapObjInstr);
        sinst->path_string = words[1];
        sinst->margin      = margin;
        sinst->file_name   = words[2];
    }
    return sinst;
}

Script::InstrPtr Script::ParseStage_(const StrVec &words) {
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

Script::InstrPtr Script::ParseState_(const StrVec &words) {
    StateInstrPtr sinst;
    const StrVec settings{
        "cursor", "headset", "mod", "tooltips", "touch", "video",
    };
    if (words.size() != 3U) {
        Error_("Bad syntax for cursor instruction");
    }
    else if (! Util::Contains(settings, words[1])) {
        Error_("Invalid setting for state instruction: " + words[1]);
    }
    else if (words[2] != "on" && words[2] != "off") {
        Error_("Third paramter for state instruction must be 'on' or 'off'");
    }
    else {
        sinst.reset(new StateInstr);
        sinst->setting = words[1];
        sinst->is_on   = words[2] == "on";
    }
    return sinst;
}

Script::InstrPtr Script::ParseStop_(const StrVec &words) {
    StopInstrPtr sinst;
    if (words.size() != 1U) {
        Error_("Bad syntax for stop instruction");
    }
    else {
        sinst.reset(new StopInstr);
    }
    return sinst;
}

Script::InstrPtr Script::ParseView_(const StrVec &words) {
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

Script::InstrPtr Script::ParseWait_(const StrVec &words) {
    WaitInstrPtr winst;
    float        duration;
    if (words.size() != 2U) {
        Error_("Bad syntax for wait instruction");
    }
    else if (! ParseFloat_(words[1], duration)) {
        Error_("Invalid duration float for wait instruction");
    }
    else {
        winst.reset(new WaitInstr);
        winst->duration = duration;
    }
    return winst;
}
