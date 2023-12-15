#include "App/CaptureScript.h"

#include "Util/String.h"

CaptureScript::CaptureScript() {
#define REG_FUNC_(name, func) \
    RegisterInstrFunc(name, [&](const StrVec &w){ return func(w); });

    REG_FUNC_("caption",      ProcessCaption_);
    REG_FUNC_("chapter",      ProcessChapter_);
    REG_FUNC_("click",        ProcessClick_);
    REG_FUNC_("cursor",       ProcessCursor_);
    REG_FUNC_("drag",         ProcessDrag_);
    REG_FUNC_("highlight",    ProcessHighlight_);
    REG_FUNC_("highlightobj", ProcessHighlightObj_);
    REG_FUNC_("moveover",     ProcessMoveOver_);
    REG_FUNC_("moveto",       ProcessMoveTo_);
    REG_FUNC_("start",        ProcessStart_);
    REG_FUNC_("tooltips",     ProcessTooltips_);
    REG_FUNC_("wait",         ProcessWait_);

#undef REG_FUNC_
}

CaptureScript::InstrPtr CaptureScript::ProcessCaption_(const StrVec &words) {
    CaptionInstrPtr cinst;
    float           x, y, seconds;
    if (words.size() < 5U) {
        Error("Bad syntax for caption instruction");
    }
    else if (! ParseFloat01(words[1], x) || ! ParseFloat01(words[2], y) ||
             ! ParseFloat(words[3], seconds)) {
        Error("Invalid x, y, or seconds floats for caption instruction");
    }
    else {
        cinst.reset(new CaptionInstr);
        cinst->pos.Set(x, y);
        cinst->seconds = seconds;
        cinst->text =
            Util::ReplaceString(
                Util::JoinStrings(StrVec(words.begin() + 4, words.end())),
                ";", "\n");
    }
    return cinst;
}

CaptureScript::InstrPtr CaptureScript::ProcessChapter_(const StrVec &words) {
    ChapterInstrPtr cinst;
    if (words.size() < 2U) {
        Error("Bad syntax for chapter instruction");
    }
    else {
        cinst.reset(new ChapterInstr);
        cinst->title =
            Util::JoinStrings(StrVec(words.begin() + 1, words.end()));
    }
    return cinst;
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
    else if (! ParseFloat(words[1], dx) || ! ParseFloat(words[2], dy) ||
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

CaptureScript::InstrPtr CaptureScript::ProcessHighlight_(const StrVec &words) {
    HighlightInstrPtr hinst;
    float             left, bottom, width, height, seconds;
    if (words.size() != 6) {
        Error("Bad syntax for highlight instruction");
    }
    else if (! ParseFloat01(words[1], left) ||
             ! ParseFloat01(words[2], bottom) ||
             ! ParseFloat(words[3], width) || ! ParseFloat(words[4], height) ||
             ! ParseFloat(words[5], seconds)) {
        Error("Invalid float argument for highlight instruction");
    }
    else if (seconds <= 0) {
        Error("Seconds for highlight instruction must be positive");
    }
    else {
        hinst.reset(new HighlightInstr);
        hinst->rect = Range2f::BuildWithSize(Point2f(left, bottom),
                                             Vector2f(width, height));
        hinst->seconds = seconds;
    }
    return hinst;
}

CaptureScript::InstrPtr CaptureScript::ProcessHighlightObj_(
    const StrVec &words) {
    HighlightObjInstrPtr hinst;
    float seconds, margin = 0;
    if (words.size() < 3U || words.size() > 4U) {
        Error("Bad syntax for highlightobj instruction");
    }
    else if (! ParseFloat(words[2], seconds)) {
        Error("Invalid seconds float for highlightobj instruction");
    }
    else if (words.size() == 4U && ! ParseFloat(words[3], margin)) {
        Error("Invalid margin float for highlightobj instruction");
    }
    else if (seconds <= 0) {
        Error("Seconds for highlightobj instruction must be positive");
    }
    else {
        hinst.reset(new HighlightObjInstr);
        hinst->object_name = words[1];
        hinst->margin      = margin;
        hinst->seconds     = seconds;
    }
    return hinst;
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

CaptureScript::InstrPtr CaptureScript::ProcessStart_(const StrVec &words) {
    StartInstrPtr sinst;
    if (words.size() != 1U) {
        Error("Bad syntax for start instruction");
    }
    else {
        sinst.reset(new StartInstr);
    }
    return sinst;
}

CaptureScript::InstrPtr CaptureScript::ProcessTooltips_(const StrVec &words) {
    TooltipsInstrPtr tinst;
    if (words.size() != 2U || (words[1] != "on" && words[1] != "off")) {
        Error("Bad syntax for tooltips instruction");
    }
    else {
        tinst.reset(new TooltipsInstr);
        tinst->is_on = words[1] == "on";
    }
    return tinst;
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

