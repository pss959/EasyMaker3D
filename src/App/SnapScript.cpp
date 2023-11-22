#include "App/SnapScript.h"

#include <ion/math/vectorutils.h>

#include "Util/Enum.h"

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
// SnapScript functions.
// ----------------------------------------------------------------------------

SnapScript::SnapScript() {
#define REG_FUNC_(name, func) \
    RegisterInstrFunc(name, [&](const StrVec &w){ return func(w); });

    REG_FUNC_("action",   ProcessAction_);
    REG_FUNC_("click",    ProcessClick_);
    REG_FUNC_("drag",     ProcessDrag_);
    REG_FUNC_("dragp",    ProcessDragP_);
    REG_FUNC_("focus",    ProcessFocus_);
    REG_FUNC_("hand",     ProcessHand_);
    REG_FUNC_("handpos",  ProcessHandPos_);
    REG_FUNC_("headset",  ProcessHeadset_);
    REG_FUNC_("hover",    ProcessHover_);
    REG_FUNC_("key",      ProcessKey_);
    REG_FUNC_("load",     ProcessLoad_);
    REG_FUNC_("mod",      ProcessMod_);
    REG_FUNC_("select",   ProcessSelect_);
    REG_FUNC_("settings", ProcessSettings_);
    REG_FUNC_("snap",     ProcessSnap_);
    REG_FUNC_("snapobj",  ProcessSnapObj_);
    REG_FUNC_("stage",    ProcessStage_);
    REG_FUNC_("stop",     ProcessStop_);
    REG_FUNC_("touch",    ProcessTouch_);
    REG_FUNC_("view",     ProcessView_);

#undef REG_FUNC_
}

SnapScript::InstrPtr SnapScript::ProcessAction_(const StrVec &words) {
    ActionInstrPtr ainst;
    Action         action;
    if (words.size() != 2U) {
        Error("Bad syntax for action instruction");
    }
    else if (! Util::EnumFromString(words[1], action)) {
        Error("Unknown action name for action instruction");
    }
    else {
        ainst.reset(new ActionInstr);
        ainst->action = action;
    }
    return ainst;
}

SnapScript::InstrPtr SnapScript::ProcessClick_(const StrVec &words) {
    ClickInstrPtr cinst;
    float x, y;
    if (words.size() != 3U) {
        Error("Bad syntax for click instruction");
    }
    else if (! ParseFloat01(words[1], x) || ! ParseFloat01(words[2], y)) {
        Error("Invalid x or y floats for click instruction");
    }
    else {
        cinst.reset(new ClickInstr);
        cinst->pos.Set(x, y);
    }
    return cinst;
}

SnapScript::InstrPtr SnapScript::ProcessDrag_(const StrVec &words) {
    DragInstrPtr dinst;
    float x0, y0, x1, y1;
    size_t count = 0;
    if (words.size() < 5U || words.size() > 6U) {
        Error("Bad syntax for drag instruction");
    }
    else if (! ParseFloat01(words[1], x0) || ! ParseFloat01(words[2], y0) ||
             ! ParseFloat01(words[3], x1) || ! ParseFloat01(words[4], y1)) {
        Error("Invalid x or y floats for drag instruction");
    }
    else if (words.size() == 6U && ! ParseN(words[5], count)) {
        Error("Invalid count for drag instruction");
    }
    else {
        dinst.reset(new DragInstr);
        dinst->pos0.Set(x0, y0);
        dinst->pos1.Set(x1, y1);
        dinst->count = count;
    }
    return dinst;
}

SnapScript::InstrPtr SnapScript::ProcessDragP_(const StrVec &words) {
    using DIPhase = DragPInstr::Phase;

    DragPInstrPtr dinst;
    float x, y;
    if (words.size() != 4U) {
        Error("Bad syntax for dragp instruction");
    }
    else if (words[1] != "start" &&
             words[1] != "continue" &&
             words[1] != "end") {
        Error("Invalid phase (start/continue/end) for dragp instruction");
    }
    else if (! ParseFloat01(words[2], x) || ! ParseFloat01(words[3], y)) {
        Error("Invalid x or y floats for dragp instruction");
    }
    else {
        dinst.reset(new DragPInstr);
        dinst->phase = words[1] == "start" ? DIPhase::kStart :
            words[1] == "continue" ? DIPhase::kContinue : DIPhase::kEnd;
        dinst->pos.Set(x, y);
    }
    return dinst;
}

SnapScript::InstrPtr SnapScript::ProcessFocus_(const StrVec &words) {
    FocusInstrPtr finst;
    if (words.size() != 2U) {
        Error("Bad syntax for focus instruction");
    }
    else {
        finst.reset(new FocusInstr);
        finst->pane_name = words[1];
    }
    return finst;
}

SnapScript::InstrPtr SnapScript::ProcessHand_(const StrVec &words) {
    HandInstrPtr hinst;
    Vector3f     pos, dir;
    if (words.size() != 3U) {
         Error("Bad syntax for hand instruction");
    }
    else if (words[1] != "L" && words[1] != "R") {
        Error("Invalid hand (L/R) for hand instruction");
    }
    else if (words[2] != "Oculus_Touch" &&
             words[2] != "Vive" &&
             words[2] != "None") {
        Error("Invalid controller type for hand instruction");
    }
    else {
        hinst.reset(new HandInstr);
        hinst->hand = words[1] == "L" ? Hand::kLeft : Hand::kRight;
        hinst->controller = words[2];
    }
    return hinst;
}

SnapScript::InstrPtr SnapScript::ProcessHandPos_(const StrVec &words) {
    HandPosInstrPtr hinst;
    Vector3f        pos;
    Vector3f        laser_dir;
    Vector3f        guide_dir;
    if (words.size() != 11U) {
         Error("Bad syntax for handpos instruction");
    }
    else if (words[1] != "L" && words[1] != "R") {
        Error("Invalid hand (L/R) for handpos instruction");
    }
    else if (! ParseVector3f(words, 2, pos)) {
        Error("Invalid position floats for handpos instruction");
    }
    else if (! ParseVector3f(words, 5, laser_dir)) {
        Error("Invalid laser direction floats for handpos instruction");
    }
    else if (! ParseVector3f(words, 8, guide_dir)) {
        Error("Invalid guide direction floats for handpos instruction");
    }
    else {
        hinst.reset(new HandPosInstr);
        hinst->hand = words[1] == "L" ? Hand::kLeft : Hand::kRight;
        hinst->pos = Point3f(pos);
        hinst->rot = ComputeHandRotation_(hinst->hand, laser_dir, guide_dir);
    }
    return hinst;
}

SnapScript::InstrPtr SnapScript::ProcessHeadset_(const StrVec &words) {
    HeadsetInstrPtr hinst;
    if (words.size() != 2U || (words[1] != "on" && words[1] != "off")) {
        Error("Bad syntax for headset instruction");
    }
    else {
        hinst.reset(new HeadsetInstr);
        hinst->is_on = words[1] == "on";
    }
    return hinst;
}

SnapScript::InstrPtr SnapScript::ProcessHover_(const StrVec &words) {
    HoverInstrPtr hinst;
    float x, y;
    if (words.size() != 3U) {
        Error("Bad syntax for hover instruction");
    }
    else if (! ParseFloat01(words[1], x) || ! ParseFloat01(words[2], y)) {
        Error("Invalid x or y floats for hover instruction");
    }
    else {
        hinst.reset(new HoverInstr);
        hinst->pos.Set(x, y);
    }
    return hinst;
}

SnapScript::InstrPtr SnapScript::ProcessKey_(const StrVec &words) {
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

SnapScript::InstrPtr SnapScript::ProcessLoad_(const StrVec &words) {
    LoadInstrPtr linst;
    if (words.size() > 2U) {
        Error("Bad syntax for load instruction");
    }
    else {
        linst.reset(new LoadInstr);
        if (words.size() == 2U)
            linst->file_name = words[1];
    }
    return linst;
}

SnapScript::InstrPtr SnapScript::ProcessMod_(const StrVec &words) {
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

SnapScript::InstrPtr SnapScript::ProcessSelect_(const StrVec &words) {
    // No names is a valid selection (deselects all).
    SelectInstrPtr sinst(new SelectInstr);
    sinst->names.insert(sinst->names.begin(), words.begin() + 1, words.end());
    return sinst;
}

SnapScript::InstrPtr SnapScript::ProcessSettings_(const StrVec &words) {
    SettingsInstrPtr sinst;
    if (words.size() != 2U) {
        Error("Bad syntax for settings instruction");
    }
    else {
        sinst.reset(new SettingsInstr);
        sinst->file_name = words[1];
    }
    return sinst;
}

SnapScript::InstrPtr SnapScript::ProcessSnap_(const StrVec &words) {
    SnapInstrPtr sinst;
    float x, y, w, h;
    if (words.size() != 6U) {
        Error("Bad syntax for snap instruction");
    }
    else if (! ParseFloat01(words[1], x) || ! ParseFloat01(words[2], y) ||
             ! ParseFloat01(words[3], w) || ! ParseFloat01(words[4], h)) {
        Error("Invalid rectangle floats for snap instruction");
    }
    else if (x + w > 1.f || y + h > 1.f) {
        Error("Rectangle is out of bounds for snap instruction");
    }
    else {
        sinst.reset(new SnapInstr);
        sinst->rect.SetWithSize(Point2f(x, y), Vector2f(w, h));
        sinst->file_name = words[5];
    }
    return sinst;
}

SnapScript::InstrPtr SnapScript::ProcessSnapObj_(const StrVec &words) {
    SnapObjInstrPtr sinst;
    float margin = 0;
    if (words.size() < 3U || words.size() > 4U) {
        Error("Bad syntax for snapobj instruction");
    }
    else if (words.size() == 4U && ! ParseFloat(words[3], margin)) {
        Error("Invalid margin float for snapobj instruction");
    }
    else {
        sinst.reset(new SnapObjInstr);
        sinst->object_name = words[1];
        sinst->margin = margin;
        sinst->file_name = words[2];
    }
    return sinst;
}

SnapScript::InstrPtr SnapScript::ProcessStage_(const StrVec &words) {
    StageInstrPtr sinst;
    float scale, angle;
    if (words.size() != 3U) {
        Error("Bad syntax for stage instruction");
    }
    else if (! ParseFloat(words[1], scale) || ! ParseFloat(words[2], angle)) {
        Error("Invalid scale/rotation data for stage instruction");
    }
    else {
        sinst.reset(new StageInstr);
        sinst->scale = scale;
        sinst->angle = Anglef::FromDegrees(angle);
    }
    return sinst;
}

SnapScript::InstrPtr SnapScript::ProcessStop_(const StrVec &words) {
    StopInstrPtr sinst;
    if (words.size() != 1U) {
        Error("Bad syntax for stop instruction");
    }
    else {
        sinst.reset(new StopInstr);
    }
    return sinst;
}

SnapScript::InstrPtr SnapScript::ProcessTouch_(const StrVec &words) {
    TouchInstrPtr tinst;
    if (words.size() != 2U || (words[1] != "on" && words[1] != "off")) {
        Error("Bad syntax for touch instruction");
    }
    else {
        tinst.reset(new TouchInstr);
        tinst->is_on = words[1] == "on";
    }
    return tinst;
}

SnapScript::InstrPtr SnapScript::ProcessView_(const StrVec &words) {
    ViewInstrPtr vinst;
    Vector3f     dir;
    if (words.size() != 4U) {
        Error("Bad syntax for view instruction");
    }
    else if (! ParseVector3f(words, 1, dir)) {
        Error("Invalid direction floats for view instruction");
    }
    else {
        ion::math::Normalize(&dir);
        vinst.reset(new ViewInstr);
        vinst->dir = dir;
    }
    return vinst;
}
