#pragma once

#include <memory>

#include "App/ScriptedApp.h"

class VideoWriter;
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
        int  fps       = 30;     ///< Frames per second (default 30).
    };

    CaptureScriptApp();
    virtual ~CaptureScriptApp();

    virtual bool Init(const OptionsPtr &options,
                      const ScriptBasePtr &script) override;

  protected:
    virtual bool ProcessInstruction(const ScriptBase::Instr &instr) override;
    virtual void InstructionsDone() override;
    virtual void FrameDone() override;

  private:
    // Handler used to update the fake cursor.
    DECL_SHARED_PTR(CursorHandler_);

    CursorHandler_Ptr            handler_;       ///< Updates the fake cursor.
    SG::NodePtr                  cursor_;        ///< Fake cursor for video.
    Point2f                      cursor_pos_;    ///< Current cursor position.
    std::unique_ptr<VideoWriter> video_writer_;  ///< Creates and writes video.

    const Options & GetOptions_() const;

    /// Adds events to move the cursor to be over the center of the named
    /// object over the given duration in seconds.
    void MoveCursorOver_(const Str &object_name, float seconds);

    /// Adds events to move the cursor to the given position in normalized
    /// window coordinates over the given duration in seconds.
    void MoveCursorTo_(const Point2f &pos, float seconds);

    /// Moves the fake cursor to the given position in normalized window
    /// coordinates.
    void MoveFakeCursorTo_(const Point2f &pos);

    /// Returns a Frustum representing the current camera view.
    Frustum GetFrustum() const;
};
