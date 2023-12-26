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
        Str  format    = "webm";  ///< Video format as a string.
        int  fps       = 30;      ///< Frames per second (default 30).
        bool nocapture = false;   ///< Don't create a capture video.
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
    // Handler used to allow pausing.
    DECL_SHARED_PTR(PauseHandler_);

    /// Struct containing information for a caption.
    struct Caption_ {
        SG::NodePtr       node;  ///< Root node for caption.
        SG::NodePtr       bg;    ///< Node with background rectangle.
        SG::TextNodePtr   text;  ///< TextNode displaying text.
        Point2f           pos;   ///< Last window position of caption.
    };

    /// This struct is used to fade items (such as captions or the highlight
    /// rectangle) in and out over a duration.
    struct FadeData_ {
        float duration = 0;   ///< Fade-in/out duration in seconds.
        float elapsed  = 0;   ///< Elapsed time in seconds.
    };

    CursorHandler_Ptr cursor_handler_;  ///< Updates the fake cursor.
    PauseHandler_Ptr  pause_handler_;   ///< Handles pausing.
    SG::NodePtr cursor_;                ///< Fake cursor for video.
    Point2f     cursor_pos_;            ///< Current cursor position.
    Caption_    caption_;               ///< Caption information.
    SG::NodePtr highlight_;             ///< Displays highlight rectangle.

    bool instructions_done_ = false;

    /// Creates and writes a video to a file.
    std::unique_ptr<VideoWriter> video_writer_;

    /// This is set to true when a start instruction is processed.
    bool is_capturing_ = false;

    FadeData_ caption_fade_data_;    ///< Caption fading/visibility.
    FadeData_ highlight_fade_data_;  ///< Highlight rectangle fading/visibility.

    const Options & GetOptions_() const;

    /// Displays a caption with the given text at the given lower-left position
    /// for the given number of seconds.
    void DisplayCaption_(const Str &text, const Point2f &pos, float seconds);

    /// Displays a highlight rectangle for the given number of seconds.
    void DisplayHighlight_(const Range2f &rect, float seconds);

    /// Adds events to drag from the current cursor position by the given
    /// motion vector over the given duration in seconds.
    void DragTo_(const Vector2f &motion, float seconds, const Str &button);

    /// Adds events to move the cursor to be over the center of the named
    /// object over the given duration in seconds.
    void MoveOver_(const Str &object_name, float seconds);

    /// Adds events to move the cursor to the given position in normalized
    /// window coordinates over the given duration in seconds.
    void MoveTo_(const Point2f &pos, float seconds);

    /// Moves the fake cursor to the given position in normalized window
    /// coordinates.
    void MoveFakeCursorTo_(const Point2f &pos);

    /// Updates fade-in/out for the caption.
    void UpdateCaption_();

    /// Updates fade-in/out for the highlight rectangle.
    void UpdateHighlight_();

    /// Updates fade-in/out for \p node using \p fade_data. Returns the alpha
    /// to use for the node.
    float UpdateFade_(SG::Node &node, FadeData_ &fade_data);

    /// Returns a 3D point on the image plane in front of the camera for the
    /// given normalized window point.
    Point3f GetImagePlanePoint_(const Point2f &pos);

    /// Returns a Frustum representing the current camera view.
    Frustum GetFrustum() const;
};
