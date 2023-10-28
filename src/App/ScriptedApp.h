#pragma once

#include <string>
#include <vector>

#include "App/Application.h"
#include "App/SnapScript.h"
#include "Math/Types.h"
#include "Util/Memory.h"

class Selection;

/// The ScriptedApp is derived from Application and adds processing of a
/// read-in SnapScript that specifies what to do.
///
/// \ingroup App
class ScriptedApp : public Application {
  public:
    /// This struct adds some additional options.
    struct Options : public Application::Options {
        SnapScript script;
        bool       nosnap = false;  ///< Ignore image snapping instructions.
        bool       remain = false;  ///< Leave the window up after.
        bool       report = false;  ///< Report each instruction when executed.
    };

    bool Init(const Options &options);

    virtual bool ProcessFrame(size_t render_count, bool force_poll) override;

    // Make this available to applications.
    using Application::GetContext;

  private:
    class Emitter_;
    DECL_SHARED_PTR(Emitter_);

    Options         options_;            ///< Set in Init().
    Vector2i        window_size_;        ///< From Options.
    size_t          cur_instruction_ = 0;
    Emitter_Ptr     emitter_;  ///< Simulates mouse and key events.

    bool ProcessInstruction_(const SnapScript::Instr &instr);
    bool LoadSession_(const Str &file_name);
    bool FocusPane_(const Str &pane_name);
    bool SetHand_(Hand hand, const Str &controller_type);
    void SetTouchMode_(bool is_on);
    bool TakeSnapshot_(const Range2f &rect, const Str &file_name);
    bool GetObjRect_(const Str &object_name, float margin, Range2f &rect);
    void BuildSelection_(const StrVec &names, Selection &selection);

    template <typename T>
    const T & GetTypedInstr_(const SnapScript::Instr &instr) {
        return static_cast<const T &>(instr);
    }
};
