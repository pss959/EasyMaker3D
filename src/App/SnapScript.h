#pragma once

#include "App/ScriptBase.h"
#include "Base/Event.h"
#include "Enums/Action.h"
#include "Enums/Hand.h"
#include "Math/Types.h"
#include "Util/Memory.h"

/// The SnapScript class is used by the snapimage application to read and store
/// a script of instructions used to generate images for public documentation.
///
/// A script contains any number of lines with one of the following
/// instructions:
///
/// \htmlinclude SnapScript.html
///
/// \ingroup App
class SnapScript : public ScriptBase {
  public:
    struct ActionInstr : public Instr {
        Action    action;
    };
    struct ClickInstr : public Instr {
        Point2f   pos;
    };
    struct DragInstr : public Instr {
        Point2f   pos0, pos1;
        size_t    count = 0;
    };
    struct DragPInstr : public Instr {
        enum class Phase { kStart, kContinue, kEnd };
        Phase     phase;
        Point2f   pos;
    };
    struct FocusInstr : public Instr {
        Str       pane_name;
    };
    struct HandInstr : public Instr {
        Hand      hand;
        Str       controller;
    };
    struct HandPosInstr : public Instr {
        Hand      hand;
        Point3f   pos;
        Rotationf rot;
    };
    struct HeadsetInstr : public Instr {
        bool      is_on;
    };
    struct HoverInstr : public Instr {
        Point2f   pos;
    };
    struct KeyInstr : public Instr {
        Event::Modifiers modifiers;
        Str              key_name;
    };
    struct LoadInstr : public Instr {
        Str       file_name;
    };
    struct ModInstr : public Instr {
        bool      is_on;
    };
    struct SelectInstr : public Instr {
        StrVec    names;
    };
    struct SettingsInstr : public Instr {
        Str       file_name;
    };
    struct SnapInstr : public Instr {
        Range2f   rect;
        Str       file_name;
    };
    struct SnapObjInstr : public Instr {
        Str       object_name;
        float     margin;
        Str       file_name;
    };
    struct StageInstr : public Instr {
        float     scale;
        Anglef    angle;
    };
    struct StopInstr : public Instr {
        // No data.
    };
    struct TouchInstr : public Instr {
        bool      is_on;
    };
    struct ViewInstr : public Instr {
        Vector3f  dir;
    };

    DECL_SHARED_PTR(ActionInstr);
    DECL_SHARED_PTR(ClickInstr);
    DECL_SHARED_PTR(DragInstr);
    DECL_SHARED_PTR(DragPInstr);
    DECL_SHARED_PTR(FocusInstr);
    DECL_SHARED_PTR(HandInstr);
    DECL_SHARED_PTR(HandPosInstr);
    DECL_SHARED_PTR(HeadsetInstr);
    DECL_SHARED_PTR(HoverInstr);
    DECL_SHARED_PTR(KeyInstr);
    DECL_SHARED_PTR(LoadInstr);
    DECL_SHARED_PTR(ModInstr);
    DECL_SHARED_PTR(SelectInstr);
    DECL_SHARED_PTR(SettingsInstr);
    DECL_SHARED_PTR(SnapInstr);
    DECL_SHARED_PTR(SnapObjInstr);
    DECL_SHARED_PTR(StageInstr);
    DECL_SHARED_PTR(StopInstr);
    DECL_SHARED_PTR(TouchInstr);
    DECL_SHARED_PTR(ViewInstr);

    /// The constructor registers all instruction-processing functions with the
    /// base class.
    SnapScript();

  private:
    InstrPtr ProcessAction_(const StrVec &words);
    InstrPtr ProcessClick_(const StrVec &words);
    InstrPtr ProcessDrag_(const StrVec &words);
    InstrPtr ProcessDragP_(const StrVec &words);
    InstrPtr ProcessFocus_(const StrVec &words);
    InstrPtr ProcessHand_(const StrVec &words);
    InstrPtr ProcessHandPos_(const StrVec &words);
    InstrPtr ProcessHeadset_(const StrVec &words);
    InstrPtr ProcessHover_(const StrVec &words);
    InstrPtr ProcessKey_(const StrVec &words);
    InstrPtr ProcessLoad_(const StrVec &words);
    InstrPtr ProcessMod_(const StrVec &words);
    InstrPtr ProcessSelect_(const StrVec &words);
    InstrPtr ProcessSettings_(const StrVec &words);
    InstrPtr ProcessSnap_(const StrVec &words);
    InstrPtr ProcessSnapObj_(const StrVec &words);
    InstrPtr ProcessStage_(const StrVec &words);
    InstrPtr ProcessStop_(const StrVec &words);
    InstrPtr ProcessTouch_(const StrVec &words);
    InstrPtr ProcessView_(const StrVec &words);
};
