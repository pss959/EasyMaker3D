#pragma once

#include <functional>
#include <map>
#include <memory>
#include <vector>

#include "App/Application.h"
#include "App/Script.h"
#include "Math/Types.h"
#include "SG/NodePath.h"
#include "Util/Memory.h"

class  Args;
class  FilePath;
struct Frustum;
class  Selection;
DECL_SHARED_PTR(ScriptEmitter);
namespace SG {
DECL_SHARED_PTR(Node);
DECL_SHARED_PTR(TextNode);
}

/// ScriptedApp is a derived Application class that adds processing of a
/// read-in script that specifies what to do.
///
/// \ingroup App
class ScriptedApp : public Application {
  public:
    /// This struct adds some additional options.
    struct Options : public Application::Options {
        virtual ~Options() {}  // Makes this polymorphic.

        Str  vidformat = "webm";  ///< Video format as a string.
        bool dryrun    = false;   ///< Do not create images or videos.
        bool remain    = false;   ///< Keep the window after the script is done.
        bool report    = false;   ///< Report each instruction when executed.
        int  fps       = 30;      ///< Frames per second (default 30).
    };

    ScriptedApp();
    virtual ~ScriptedApp();

    /// Initializes Options from command-line arguments.
    void InitOptions(const Args &args);

    /// Returns the Options.
    Options & GetOptions() { return options_; }

    /// Initializes the ScriptedApp. Returns false if anything goes wrong.
    bool InitApp();

    /// Processes the Script. The \p do_video flag indicates whether video may
    /// be captured. Returns false if anything goes wrong.
    bool ProcessScript(const FilePath &script_path, bool do_video);

    /// Redefines this to add script processing during frames.
    virtual bool ProcessFrame(size_t render_count, bool force_poll) override;

    // Provide context access for testing.
    using Application::GetContext;

  protected:
    /// Redefines this to return the Emitter modified mode state.
    virtual bool IsInModifiedMode() const override;

  private:
    /// Struct containing information for a caption.
    struct Caption_ {
        SG::NodePtr       node;  ///< Root node for caption.
        SG::NodePtr       bg;    ///< Node with background rectangle.
        SG::TextNodePtr   text;  ///< TextNode displaying text.
        Point2f           pos;   ///< Last window position of caption.
    };

    // Used to fake a file system for panels.
    class MockFilePathList_;

    // This is used for video-only processing.
    class Video_;

    using InstrFunc_ = std::function<bool(const Script::Instr &)>;
    using FuncMap_   = std::map<Str, InstrFunc_>;

    // Handler used to update the fake cursor.
    DECL_SHARED_PTR(CursorHandler_);
    // Handler used to allow pausing.
    DECL_SHARED_PTR(PauseHandler_);

    /// This maps instruction names to functions.
    FuncMap_         func_map_;

    Options          options_;
    Script           script_;
    ScriptEmitterPtr emitter_;      ///< Used to simulate mouse and key events.

    bool    is_paused_       = false;
    size_t  cur_instruction_ = 0;
    Point2f drag_start_pos_;

    CursorHandler_Ptr cursor_handler_;  ///< Updates the fake cursor.
    PauseHandler_Ptr  pause_handler_;   ///< Handles pausing.
    SG::NodePtr cursor_;                ///< Fake cursor for video.
    Point2f     cursor_pos_;            ///< Current cursor position.
    Caption_    caption_;               ///< Caption information.
    SG::NodePtr highlight_;             ///< Displays highlight rectangle.

    std::unique_ptr<MockFilePathList_> mock_fpl_;  ///< Simulates files.

    std::unique_ptr<Video_> video_;  ///< Used only when capturing video.

    /// \name Initialization.
    ///@{

    /// Initializes everything. Returns false on error.
    bool Init_();

    void InitControllers_();
    void InitMockFilePathList_();
    void InitScene_();
    void InitHandlers_();
    ///@}

    /// Processes one instruction.
    bool ProcessInstruction_(const Script::Instr &instr);

    /// \name Specific instruction processing.
    ///@{
    bool ProcessAction_(const Script::ActionInstr &instr);
    bool ProcessCaption_(const Script::CaptionInstr &instr);
    bool ProcessClick_(const Script::ClickInstr &instr);
    bool ProcessDrag_(const Script::DragInstr &instr);
    bool ProcessDragStart_(const Script::DragStartInstr &instr);
    bool ProcessDragEnd_(const Script::DragEndInstr &instr);
    bool ProcessFocus_(const Script::FocusInstr &instr);
    bool ProcessHand_(const Script::HandInstr &instr);
    bool ProcessHandPos_(const Script::HandPosInstr &instr);
    bool ProcessHighlight_(const Script::HighlightInstr &instr);
    bool ProcessKey_(const Script::KeyInstr &instr);
    bool ProcessLoad_(const Script::LoadInstr &instr);
    bool ProcessMoveOver_(const Script::MoveOverInstr &instr);
    bool ProcessMoveTo_(const Script::MoveToInstr &instr);
    bool ProcessSection_(const Script::SectionInstr &instr);
    bool ProcessSelect_(const Script::SelectInstr &instr);
    bool ProcessSettings_(const Script::SettingsInstr &instr);
    bool ProcessSnap_(const Script::SnapInstr &instr);
    bool ProcessSnapObj_(const Script::SnapObjInstr &instr);
    bool ProcessStage_(const Script::StageInstr &instr);
    bool ProcessState_(const Script::StateInstr &instr);
    bool ProcessStop_(const Script::StopInstr &instr);
    bool ProcessView_(const Script::ViewInstr &instr);
    bool ProcessWait_(const Script::WaitInstr &instr);
    ///@}

    /// Finishes main instruction processing.
    void Finish_();

    /// Returns a Frustum representing the current camera view.
    Frustum GetFrustum_() const;

    /// Returns a 3D point on the image plane in front of the camera for the
    /// given normalized window point.
    Point3f GetImagePlanePoint_(const Point2f &pos);

    /// Resolves a Node path string to an SG::NodePath.
    SG::NodePath GetNodePath_(const Str &path_string);

    /// Sets \p rect to an image-plane rectangle surrounding the Node
    /// referenced by the given path string with given margin on all
    /// sides. Prints an error message and returns false if the Node is not
    /// found.
    bool GetNodeRect_(const Str &path_string, float margin, Range2f &rect);

    /// Loads Settings from the given path, updating the SettingsManager and
    /// SessionManager. Returns false on error.
    bool LoadSettings_(const FilePath &path);

    /// Moves the fake cursor to the given position in normalized window
    /// coordinates.
    void MoveFakeCursorTo_(const Point2f &pos);

    /// Adds events to move the cursor to the given position in normalized
    /// window coordinates over the given duration in seconds.
    void MoveTo_(const Point2f &pos, float duration);

    /// Pauses or resumes processing, meaning that instructions will not be
    /// processed until unpaused. Returns true if the application is now
    /// paused. This is provided mostly for pausing video capture to inspect
    /// the scene and debug issues.
    bool PauseOrUnpause_();

    /// Turns panel touch mode on or off.
    void SetTouchMode_(bool is_on);

    /// Takes a snapshot of a rectangular region of the window, saving it to
    /// the named file in the images directory.
    bool TakeSnapshot_(const Range2f &rect, const Str &file_name);

    /// Updates the caption position, rotation, and fade if necessary.
    void UpdateCaption_();
};
