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

class Args;
class FilePath;
class Selection;
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

    /// Processes the Script named by the given FilePath. Returns false if
    /// anything goes wrong.
    bool ProcessScript(const FilePath &script_path);

  protected:
    /// Redefines this to add script processing during frames.
    virtual bool ProcessFrame(size_t render_count, bool force_poll) override;

    /// Processes one instruction.
    bool ProcessInstruction_(const Script::Instr &instr);

    /// \name Specific instruction processing.
    ///@{
    bool ProcessAction_(const Script::ActionInstr &instr);
    bool ProcessCaption_(const Script::CaptionInstr &instr);
    bool ProcessClick_(const Script::ClickInstr &instr);
    bool ProcessDrag_(const Script::DragInstr &instr);
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

    /// Lets derived classes know when instructions have all been processed.
    /// The base class defines this to do nothing.
    virtual void InstructionsDone() {}

    /// Allows derived classes to pause or resume processing, meaning that
    /// instructions will not be processed and BeginFrame()/EndFrame() will not
    /// be called until unpaused. Returns true if the application is now
    /// paused.
    bool PauseOrUnpause();

    /// Lets derived classes know when a frame is starting. The base class
    /// defines this to do nothing.
    virtual void BeginFrame() {}

    /// Lets derived classes know when a frame is ending. The base class
    /// defines this to do nothing.
    virtual void EndFrame() {}

    /// Loads Settings from the given path, updating the SettingsManager and
    /// SessionManager. Returns false on error.
    bool LoadSettings(const FilePath &path);


    /// Sets \p rect to an image-plane rectangle surrounding the Node
    /// referenced by the given path string with given margin on all
    /// sides. Prints an error message and returns false if the Node is not
    /// found.
    bool GetNodeRect(const Str &path_string, float margin, Range2f &rect);

    /// Resolves a Node path string to an SG::NodePath.
    SG::NodePath GetNodePath(const Str &path_string);

  private:
    /// Struct containing information for a caption.
    struct Caption_ {
        SG::NodePtr       node;  ///< Root node for caption.
        SG::NodePtr       bg;    ///< Node with background rectangle.
        SG::TextNodePtr   text;  ///< TextNode displaying text.
        Point2f           pos;   ///< Last window position of caption.
    };

    // This is used for video-only processing.
    struct Video_;

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

    bool   is_paused_       = false;
    size_t cur_instruction_ = 0;

    CursorHandler_Ptr cursor_handler_;  ///< Updates the fake cursor.
    PauseHandler_Ptr  pause_handler_;   ///< Handles pausing.
    SG::NodePtr cursor_;                ///< Fake cursor for video.
    Point2f     cursor_pos_;            ///< Current cursor position.
    Caption_    caption_;               ///< Caption information.
    SG::NodePtr highlight_;             ///< Displays highlight rectangle.

    std::unique_ptr<Video_> video_;  ///< Used only when capturing video.

    /// Updates the caption position, rotation, and fade if necessary.
    void UpdateCaption_();

    /// Returns a 3D point on the image plane in front of the camera for the
    /// given normalized window point.
    Point3f GetImagePlanePoint_(const Point2f &pos);

    /// Returns a Frustum representing the current camera view.
    Frustum GetFrustum() const;
};
