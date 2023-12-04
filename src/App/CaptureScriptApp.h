#pragma once

#include <memory>

#include "App/ScriptedApp.h"

class VideoWriter;
namespace SG {
DECL_SHARED_PTR(Node);
DECL_SHARED_PTR(TextNode);
}

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
    virtual void BeginFrame() override;
    virtual void EndFrame() override;

  private:
    // Handler used to update the fake cursor.
    DECL_SHARED_PTR(CursorHandler_);

    CursorHandler_Ptr            handler_;       ///< Updates the fake cursor.
    SG::NodePtr                  cursor_;        ///< Fake cursor for video.
    Point2f                      cursor_pos_;    ///< Current cursor position.
    SG::TextNodePtr              caption_;       ///< Displays caption text.
    std::unique_ptr<VideoWriter> video_writer_;  ///< Creates and writes video.

    /// \name Caption fading/visibility.
    ///@{
    float caption_seconds_         = 0;
    float caption_seconds_elapsed_ = 0;
    ///@}

    const Options & GetOptions_() const;

    /// Displays a caption with the given text at the given lower-left position
    /// for the given number of seconds.
    void DisplayCaption_(const Str &text, const Point2f &pos, float seconds);

    /// Updates the caption visibility or alpha to fade in/out depending on the
    /// number of frames left to display.
    void UpdateCaption_();

    /// Adds events to drag from the current cursor position by the given
    /// motion vector over the given duration in seconds.
    void DragTo_(const Vector2f &motion, float seconds);

    /// Adds events to move the cursor to be over the center of the named
    /// object over the given duration in seconds.
    void MoveOver_(const Str &object_name, float seconds);

    /// Adds events to move the cursor to the given position in normalized
    /// window coordinates over the given duration in seconds.
    void MoveTo_(const Point2f &pos, float seconds);

    /// Moves the fake cursor to the given position in normalized window
    /// coordinates.
    void MoveFakeCursorTo_(const Point2f &pos);

    /// Returns a 3D point on the image plane in front of the camera for the
    /// given normalized window point.
    Point3f GetImagePlanePoint_(const Point2f &pos);

    /// Returns a Frustum representing the current camera view.
    Frustum GetFrustum() const;
};
