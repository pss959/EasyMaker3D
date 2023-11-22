#pragma once

#include "App/ScriptBase.h"
#include "Math/Types.h"
#include "Util/Memory.h"

/// The CaptureScript class is used by the capturevideo application to read and
/// store a script of instructions used to generate videos for public
/// documentation. These instructions move a visible (fake) cursor and click on
/// or drag various Widgets. Timing is controlled with duration arguments for
/// the instructions.
///
/// A script contains any number of lines with one of the following
/// instructions:
///
/// \htmlinclude CaptureScript.html
///
/// \ingroup App
class CaptureScript : public ScriptBase {
  public:
    struct ClickInstr : public Instr {
        // No fields.
    };
    struct CursorInstr : public Instr {
        bool    is_on;
    };
    struct DragInstr : public Instr {
        Point2f motion;
        float   seconds;
    };
    struct ModInstr : public Instr {
        bool    is_on;
    };
    struct MoveOverInstr : public Instr {
        Str     object_name;
        float   seconds;
    };
    struct MoveToInstr : public Instr {
        Point2f pos;
        float   seconds;
    };
    struct WaitInstr : public Instr {
        float   seconds;
    };

    DECL_SHARED_PTR(ClickInstr);
    DECL_SHARED_PTR(CursorInstr);
    DECL_SHARED_PTR(DragInstr);
    DECL_SHARED_PTR(ModInstr);
    DECL_SHARED_PTR(MoveOverInstr);
    DECL_SHARED_PTR(MoveToInstr);
    DECL_SHARED_PTR(WaitInstr);

    /// The constructor registers all instruction-processing functions with the
    /// base class.
    CaptureScript();

  private:
    InstrPtr ProcessClick_(const StrVec &words);
    InstrPtr ProcessCursor_(const StrVec &words);
    InstrPtr ProcessDrag_(const StrVec &words);
    InstrPtr ProcessMod_(const StrVec &words);
    InstrPtr ProcessMoveOver_(const StrVec &words);
    InstrPtr ProcessMoveTo_(const StrVec &words);
    InstrPtr ProcessWait_(const StrVec &words);
};
