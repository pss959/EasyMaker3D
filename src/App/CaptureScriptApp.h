#pragma once

#include "App/Application.h"
#include "App/CaptureScript.h"
#include "Math/Types.h"

namespace SG { DECL_SHARED_PTR(Node); }

/// CaptureScriptApp is derived from Application and adds processing of a
/// read-in CaptureScript that specifies what to do.
///
/// \ingroup App
class CaptureScriptApp : public Application {
  public:
    /// This struct adds some additional options.
    struct Options : public Application::Options {
        CaptureScript script;
        bool nocapture = false;  ///< Don't create a capture video.
        bool remain    = false;  ///< Leave the window up after.
        bool report    = false;  ///< Report each instruction when executed.
    };

    bool Init(const Options &options);

    virtual bool ProcessFrame(size_t render_count, bool force_poll) override;

    // Make this available to applications.
    using Application::GetContext;

  private:
    Options     options_;            ///< Set in Init().
    Vector2i    window_size_;        ///< From Options.
    SG::NodePtr cursor_;             ///< Fake cursor for video.
    size_t      cur_instruction_ = 0;

    bool ProcessInstruction_(const CaptureScript::Instr &instr);

    template <typename T>
    const T & GetTypedInstr_(const CaptureScript::Instr &instr) {
        return static_cast<const T &>(instr);
    }
};
