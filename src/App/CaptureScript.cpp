#include "App/CaptureScript.h"

CaptureScript::CaptureScript() {
#define REG_FUNC_(name, func) \
    RegisterInstrFunc(name, [&](const StrVec &w){ return func(w); });

    REG_FUNC_("click",    ProcessClick_);
    REG_FUNC_("cursor",   ProcessCursor_);
    REG_FUNC_("drag",     ProcessDrag_);
    REG_FUNC_("mod",      ProcessMod_);
    REG_FUNC_("moveover", ProcessMoveOver_);
    REG_FUNC_("moveto",   ProcessMoveTo_);
    REG_FUNC_("wait",     ProcessWait_);

#undef REG_FUNC_
}

CaptureScript::InstrPtr CaptureScript::ProcessClick_(const StrVec &words) {
    ClickInstrPtr cinst;
    if (words.size() != 1U) {
        Error("Bad syntax for click instruction");
    }
    else {
        cinst.reset(new ClickInstr);
    }
    return cinst;
}

CaptureScript::InstrPtr CaptureScript::ProcessCursor_(const StrVec &words) {
    CursorInstrPtr cinst;
    if (words.size() != 2U || (words[1] != "on" && words[1] != "off")) {
        Error("Bad syntax for cursor instruction");
    }
    else {
        cinst.reset(new CursorInstr);
        cinst->is_on = words[1] == "on";
    }
    return cinst;
}

CaptureScript::InstrPtr CaptureScript::ProcessDrag_(const StrVec &words) {
    DragInstrPtr dinst;
    float        dx, dy, seconds;
    if (words.size() != 4) {
        Error("Bad syntax for drag instruction");
    }
    else if (! ParseFloat01(words[1], dx) || ! ParseFloat01(words[2], dy) ||
             ! ParseFloat(words[3], seconds)) {
        Error("Invalid dx, dy, or seconds floats for drag instruction");
    }
    else if (seconds <= 0) {
        Error("Seconds for drag instruction must be positive");
    }
    else {
        dinst.reset(new DragInstr);
        dinst->motion.Set(dx, dy);
        dinst->seconds = seconds;
    }
    return dinst;
}

CaptureScript::InstrPtr CaptureScript::ProcessMod_(const StrVec &words) {
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

CaptureScript::InstrPtr CaptureScript::ProcessMoveOver_(const StrVec &words) {
    MoveOverInstrPtr minst;
    float            seconds;
    if (words.size() != 3U) {
        Error("Bad syntax for moveover instruction");
    }
    else if (! ParseFloat(words[2], seconds)) {
        Error("Invalid seconds float for moveover instruction");
    }
    else {
        minst.reset(new MoveOverInstr);
        minst->object_name = words[1];
        minst->seconds = seconds;
    }
    return minst;
}

CaptureScript::InstrPtr CaptureScript::ProcessMoveTo_(const StrVec &words) {
    MoveToInstrPtr minst;
    float          x, y, seconds;
    if (words.size() != 4) {
        Error("Bad syntax for moveto instruction");
    }
    else if (! ParseFloat01(words[1], x) || ! ParseFloat01(words[2], y) ||
             ! ParseFloat(words[3], seconds)) {
        Error("Invalid x, y, or seconds floats for moveto instruction");
    }
    else {
        minst.reset(new MoveToInstr);
        minst->pos.Set(x, y);
        minst->seconds = seconds;
    }
    return minst;
}

CaptureScript::InstrPtr CaptureScript::ProcessWait_(const StrVec &words) {
    WaitInstrPtr winst;
    float        seconds;
    if (words.size() != 2U) {
        Error("Bad syntax for wait instruction");
    }
    else if (! ParseFloat(words[1], seconds)) {
        Error("Invalid seconds float for wait instruction");
    }
    else {
        winst.reset(new WaitInstr);
        winst->seconds = seconds;
    }
    return winst;
}

