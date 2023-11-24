#pragma once

#include "App/ScriptBase.h"
#include "App/ScriptedApp.h"
#include "Enums/Hand.h"

class Selection;

/// SnapScriptApp is derived from Application and adds processing of a read-in
/// SnapScript that specifies what to do.
///
/// \ingroup App
class SnapScriptApp : public ScriptedApp {
  public:
    /// This struct adds some additional options.
    struct Options : public ScriptedApp::Options {
        bool nosnap = false;  ///< Ignore image snapping instructions.
    };

  protected:
    virtual bool ProcessInstruction(const ScriptBase::Instr &instr) override;

  private:
    const Options & GetOptions_() const;
    bool LoadSession_(const Str &file_name);
    bool FocusPane_(const Str &pane_name);
    bool SetHand_(Hand hand, const Str &controller_type);
    void SetTouchMode_(bool is_on);
    bool TakeSnapshot_(const Range2f &rect, const Str &file_name);
    bool GetObjRect_(const Str &object_name, float margin, Range2f &rect);
    void BuildSelection_(const StrVec &names, Selection &selection);
};
