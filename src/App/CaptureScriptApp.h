#pragma once

#include "App/ScriptedApp.h"

namespace SG { DECL_SHARED_PTR(Node); }

/// CaptureScriptApp is a derived ScriptedApp that adds processing of a read-in
/// CaptureScript.
///
/// \ingroup App
class CaptureScriptApp : public ScriptedApp {
  public:
    /// This struct adds some additional options.
    struct Options : public ScriptedApp::Options {
        bool nocapture = false;  ///< Don't create a capture video.
    };

    virtual bool Init(const OptionsPtr &options,
                      const ScriptBasePtr &script) override;

  protected:
    virtual bool ProcessInstruction(const ScriptBase::Instr &instr) override;

  private:
    SG::NodePtr cursor_;  ///< Fake cursor for video.

    const Options & GetOptions_() const;

    /// Moves the fake cursor to the given position in normalized window
    /// coordinates using the given frustum.
    void MoveCursorTo_(const Frustum &frustum, const Point2f &pos);

    /// Moves the fake cursor over the center of the named object.
    void MoveCursorOver_(const Str &object_name);

    /// Returns a Frustum representing the current camera view.
    Frustum GetFrustum() const;
};
