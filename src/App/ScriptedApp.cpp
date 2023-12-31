#include "App/ScriptedApp.h"

#include <ion/math/rangeutils.h>
#include <ion/math/transformutils.h>

#include "App/ActionProcessor.h"
#include "App/Args.h"
#include "App/Script.h"
#include "App/ScriptEmitter.h"
#include "App/VideoWriter.h"
#include "Base/Event.h"
#include "Debug/Shortcuts.h"
#include "Feedback/TooltipFeedback.h"
#include "Handlers/Handler.h"
#include "Items/Controller.h"
#include "Items/Settings.h"
#include "Managers/AnimationManager.h"
#include "Managers/BoardManager.h"
#include "Managers/EventManager.h"
#include "Managers/PanelManager.h"
#include "Managers/SceneContext.h"
#include "Managers/SelectionManager.h"
#include "Managers/SessionManager.h"
#include "Managers/SettingsManager.h"
#include "Math/Intersection.h"
#include "Math/Linear.h"
#include "Models/Model.h"
#include "Models/RootModel.h"
#include "Panels/Board.h"
#include "Panels/FilePanel.h"
#include "Panels/KeyboardPanel.h"
#include "Parser/Parser.h"
#include "SG/CoordConv.h"
#include "SG/Node.h"
#include "SG/Scene.h"
#include "SG/Search.h"
#include "SG/TextNode.h"
#include "SG/VRCamera.h"
#include "SG/WindowCamera.h"
#include "Selection/Selection.h"
#include "Util/Assert.h"
#include "Util/FilePath.h"
#include "Util/FilePathList.h"
#include "Util/KLog.h"
#include "Util/Read.h"
#include "Util/Tuning.h"
#include "Util/Write.h"
#include "Viewers/Renderer.h"
#include "Widgets/Slider1DWidget.h"
#include "Widgets/StageWidget.h"

namespace {

// ----------------------------------------------------------------------------
// MockFilePathList_ class.
// ----------------------------------------------------------------------------

/// Derived FilePathList class that simulates a file system so that
/// documentation images are consistent and predictable.
class MockFilePathList_ : public FilePathList {
    /// Redefines this to simulate files.
    virtual void GetContents(StrVec &subdirs, StrVec &files,
                             const Str &extension,
                             bool include_hidden) const override;
    virtual bool IsValidDirectory(const FilePath &path) const {
        const Str fn = path.GetFileName();
        return fn.starts_with("Dir") || fn == "stl" || fn == "maker";
    }
    virtual bool IsExistingFile(const FilePath &path) const {
        return true;
    }
};

void MockFilePathList_::GetContents(StrVec &subdirs, StrVec &files,
                                    const Str &extension,
                                    bool include_hidden) const {
    const FilePath dir = GetCurrent();
    // Special case for dummy path used in doc.
    if (dir.ToString() == "/projects/maker/stl/") {
        files.push_back("Airplane.stl");
        files.push_back("Boat.stl");
        files.push_back("Car.stl");
    }
    else {
        subdirs.push_back("Dir0");
        subdirs.push_back("Dir1");
        files.push_back("FileA" + extension);
        files.push_back("FileB" + extension);
        files.push_back("FileC" + extension);
        files.push_back("FileD" + extension);
        files.push_back("FileE" + extension);
    }
}

}  // anonymous namespace

// ----------------------------------------------------------------------------
// ScriptedApp::Video_ class.
// ----------------------------------------------------------------------------

class ScriptedApp::Video_ {
  public:
    Video_(const Str &format, const int fps) :
        fps_(fps),
        video_writer_(format == "rgbmp4" ? VideoWriter::Format::kRGBMP4 :
                      format == "yuvmp4" ? VideoWriter::Format::kYUVMP4 :
                      VideoWriter::Format::kWEBM) {
    }

    /// Initializes fade-in/out for the caption.
    void InitCaptionFade(float duration) {
        caption_fade_data_.duration = duration;
        caption_fade_data_.elapsed  = 0;
    }

    /// Initializes fade-in/out for the highlight.
    void InitHighlightFade(float duration) {
        highlight_fade_data_.duration = duration;
        highlight_fade_data_.elapsed  = 0;
    }

    /// Updates fade-in/out for the caption.
    void UpdateCaptionFade(const Caption_ &caption);

    /// Updates fade-in/out for the highlight rectangle.
    void UpdateHighlightFade(SG::Node &highlight);

  private:
    /// This struct is used to fade items (such as captions or the highlight
    /// rectangle) in and out over a duration.
    struct FadeData_ {
        float duration = 0;   ///< Fade-in/out duration in seconds.
        float elapsed  = 0;   ///< Elapsed time in seconds.
    };

    const int fps_;  ///< Frames per second passed to constructor.

    /// Creates and writes a video to a file.
    VideoWriter video_writer_;

    /// This is set to true when a start instruction is processed.
    bool is_capturing_ = false;

    FadeData_ caption_fade_data_;    ///< Caption fading/visibility.
    FadeData_ highlight_fade_data_;  ///< Highlight rectangle fading/visibility.

    /// Updates fade-in/out for \p node using \p fade_data. Returns the alpha
    /// to use for the node.
    float UpdateFade_(SG::Node &node, FadeData_ &fade_data);
};

void ScriptedApp::Video_::UpdateCaptionFade(const Caption_ &caption) {
    // Note that this fades just the text, not the background.
    if (caption_fade_data_.elapsed <= caption_fade_data_.duration) {
        const float alpha = UpdateFade_(*caption.node, caption_fade_data_);
        Color c = caption.text->GetColor();
        c[3] = alpha;
        caption.text->SetTextColor(c);
    }
}

void ScriptedApp::Video_::UpdateHighlightFade(SG::Node &highlight) {
    if (highlight_fade_data_.elapsed <= highlight_fade_data_.duration) {
        const float alpha = UpdateFade_(highlight, highlight_fade_data_);
        Color c = highlight.GetBaseColor();
        c[3] = alpha;
        highlight.SetBaseColor(c);
    }
}

float ScriptedApp::Video_::UpdateFade_(SG::Node &node, FadeData_ &fade_data) {
    // Fade-in/out time.
    const float kFadeTime = .5f;

    float alpha = 1;
    if (fade_data.elapsed < kFadeTime) {
        // Just starting, so fade in.
        alpha = fade_data.elapsed / kFadeTime;
    }
    else if (fade_data.elapsed >= fade_data.duration - kFadeTime) {
        // Almost done, so fade out.
        alpha = (fade_data.duration - fade_data.elapsed) / kFadeTime;
    }

    fade_data.elapsed += 1. / fps_;

    // Completely done?
    if (fade_data.elapsed >= fade_data.duration)
        node.SetEnabled(false);

    return alpha;
}

// ----------------------------------------------------------------------------
// ScriptedApp functions.
// ----------------------------------------------------------------------------

ScriptedApp::ScriptedApp() {
    // Set up the map of instruction name to function.

    // E.g, maps REG_FUNC_(Action) to ProcessAction_(cast to ActionInstr).
#define REG_FUNC_(name)                                                       \
    func_map_[Util::ToLowerCase(#name)] = [&](const Script::Instr &instr){    \
        const auto &ti = static_cast<const Script::name ## Instr &>(instr);   \
        return Process ## name ## _(ti);                                      \
    }

    REG_FUNC_(Action);
    REG_FUNC_(Caption);
    REG_FUNC_(Click);
    REG_FUNC_(Drag);
    REG_FUNC_(Focus);
    REG_FUNC_(Hand);
    REG_FUNC_(HandPos);
    REG_FUNC_(Highlight);
    REG_FUNC_(Key);
    REG_FUNC_(Load);
    REG_FUNC_(MoveOver);
    REG_FUNC_(MoveTo);
    REG_FUNC_(Section);
    REG_FUNC_(Select);
    REG_FUNC_(Settings);
    REG_FUNC_(Snap);
    REG_FUNC_(SnapObj);
    REG_FUNC_(Stage);
    REG_FUNC_(State);
    REG_FUNC_(Stop);
    REG_FUNC_(View);
    REG_FUNC_(Wait);

#undef REG_FUNC_
}

ScriptedApp::~ScriptedApp() {}

void ScriptedApp::InitOptions(const Args &args) {
    // KLogging for debugging.
    KLogger::SetKeyString(args.GetString("--klog"));

    options_.do_ion_remote      = true;
    options_.enable_vr          = true;   // So controllers work properly.
    options_.fullscreen         = args.GetBool("--fullscreen");
    options_.remain             = args.GetBool("--remain");
    options_.report             = args.GetBool("--report");

    // Window size. Note that this must have the same aspect ratio as
    // fullscreen.
    int size_n = args.GetAsInt("--size", 1);
    if (size_n <= 0)
        size_n = 1;
    options_.window_size.Set(1024 / size_n, 552 / size_n);
}

bool ScriptedApp::ProcessScript(const FilePath &script_path) {
    // Read the script.
    if (! script_.ReadScript(script_path))
        return false;

    if (! Application::Init(options_))
        return false;

    emitter_.reset(new ScriptEmitter);
    AddEmitter(emitter_);

    const auto &context = GetContext();

    // Turn off controllers until they are specifically added.
    context.scene_context->left_controller->SetEnabled(false);
    context.scene_context->right_controller->SetEnabled(false);

#if ENABLE_DEBUG_FEATURES
    // Make sure there is no debug text visible.
    Debug::DisplayDebugText("");
#endif

    // Ignore mouse events from GLFWViewer so they do not interfere with the
    // click and drag events.
    EnableMouseMotionEvents(false);

    // No need to ask before quitting a scripted app.
    SetAskBeforeQuitting(false);

    // Set render offsets for the controllers.
    SetControllerRenderOffsets(-ScriptEmitter::kLeftControllerOffset,
                               -ScriptEmitter::kRightControllerOffset);

    // Use default settings file so that state is deterministic.
    const FilePath path("PublicDoc/settings/Settings" + TK::kDataFileExtension);
    if (! LoadSettings(path))
        return false;

    // Use the MockFilePathList_ for the FilePanel and ImportToolPanel.
    const auto set_mock = [&](const Str &panel_name){
        const auto &panel_mgr = *GetContext().panel_manager;
        auto panel = panel_mgr.GetTypedPanel<FilePanel>(panel_name);
        panel->SetFilePathList(new MockFilePathList_);
    };
    set_mock("FilePanel");
    set_mock("ImportToolPanel");

    try {
        MainLoop();
    }
    catch (AssertException &ex) {
        std::cerr << "*** Caught Assertion failure: " << ex.what() << "\n";
        std::cerr << "*** STACK:\n";
        for (const auto &s: ex.GetStackTrace())
            std::cerr << "  " << s << "\n";
        return -1;
    }
    catch (std::exception &ex) {
        std::cerr << "*** Caught exception: " << ex.what() << "\n";
        return -1;
    }

    return true;
}

bool ScriptedApp::ProcessFrame(size_t render_count, bool force_poll) {
    // If paused, just render, waiting for events.
    if (is_paused_)
        return Application::ProcessFrame(render_count, false);

    const size_t instr_count         = script_.GetInstructions().size();
    const bool events_pending        = emitter_->HasPendingEvents();
    const bool are_more_instructions = cur_instruction_ < instr_count;
    bool keep_going;
    bool was_stopped = false;

    // Let the derived class know the frame is starting.
    BeginFrame();

    // Let the base class check for exit. Force it to poll for events if there
    // are instructions left to process, there are pending emitter events, or
    // if the window is supposed to go away; don't want to wait for an event to
    // come along.
    const bool should_poll =
        are_more_instructions || events_pending || ! options_.remain;
    if (! Application::ProcessFrame(render_count, should_poll)) {
        keep_going = false;
    }

    // If there are events pending, do not process more instructions before
    // they are handled.
    else if (events_pending) {
        keep_going = true;
    }

    else if (are_more_instructions) {
        const auto &instr = *script_.GetInstructions()[cur_instruction_];
        const size_t instr_count = script_.GetInstructions().size();
        if (options_.report)
            std::cout << "  Processing " << instr.name
                      << " (instruction " << (cur_instruction_ + 1)
                      << " of " << instr_count << ") on line "
                      << instr.line_number << "\n";
        if (instr.name == "stop") {
            keep_going  = false;
            was_stopped = true;
            cur_instruction_ = instr_count;
            emitter_->FlushPendingEvents();
        }
        else {
            keep_going = ProcessInstruction_(instr);
            ++cur_instruction_;
        }
    }

    // There are no pending events and no more instructions. If not currently
    // animating, processing is done.
    else if (! GetContext().animation_manager->IsAnimating()) {
        was_stopped = true;
    }

    // Let the derived class know the frame is done.
    EndFrame();

    // If processing is done, let the derived class know and stop ignoring
    // mouse events from GLFWViewer.
    if (was_stopped) {
        InstructionsDone();
        EnableMouseMotionEvents(true);
        keep_going = options_.remain;
    }

    return keep_going;
}

bool ScriptedApp::ProcessInstruction_(const Script::Instr &instr) {
    const auto it = func_map_.find(instr.name);
    ASSERTM(it != func_map_.end(), "Bad instruction name " + instr.name);
    return it->second(instr);

#if XXXX
    auto &context = GetContext();

    if (instr.name == "action") {
        const auto &ainst = GetTypedInstr_<Script::ActionInstr>(instr);
    }
    else if (instr.name == "caption") {
        const auto &cinst = GetTypedInstr_<Script::CaptionInstr>(instr);
    }
    else if (instr.name == "click") {
        emitter_.AddClick(cursor_pos_);
    }
    else if (instr.name == "drag") {
        const auto &dinst = GetTypedInstr_<Script::DragInstr>(instr);
        DragTo_(dinst.motion, dinst.duration, dinst.button);
    }
    else if (instr.name == "focus") {
        const auto &finst = GetTypedInstr_<Script::FocusInstr>(instr);
        FocusPane_(finst.pane_name);
    }
    else if (instr.name == "hand") {
        const auto &hinst = GetTypedInstr_<Script::HandInstr>(instr);
        if (! SetHand_(hinst.hand, hinst.controller))
            return false;
    }
    else if (instr.name == "handpos") {
        const auto &hinst = GetTypedInstr_<Script::HandPosInstr>(instr);
        emitter.AddControllerPos(hinst.hand, hinst.pos, hinst.rot);
    }
    else if (instr.name == "headset") {
        const auto &hinst = GetTypedInstr_<Script::HeadsetInstr>(instr);
        emitter.AddHeadsetButton(hinst.is_on);
    }



    else if (instr.name == "cursor") {
        const auto &cinst = GetTypedInstr_<Script::CursorInstr>(instr);
        cursor_->SetEnabled(cinst.is_on);
    }
    else if (instr.name == "highlight") {
        const auto &hinst = GetTypedInstr_<Script::HighlightInstr>(instr);
        Range2f rect;
        if (! GetNodeRect(hinst.path_string, hinst.margin, rect))
            return false;
        DisplayHighlight_(rect, hinst.duration);
    }
    else if (instr.name == "key") {
        const auto &kinst = GetTypedInstr_<Script::KeyInstr>(instr);
        GetEmitter().AddKey(kinst.key_string);
    }
    else if (instr.name == "mod") {
        const auto &minst = GetTypedInstr_<Script::ModInstr>(instr);
        GetEmitter().SetModifiedMode(minst.is_on);
    }
    else if (instr.name == "moveover") {
        const auto &minst = GetTypedInstr_<Script::MoveOverInstr>(instr);
        MoveOver_(minst.path_string, minst.duration);
    }
    else if (instr.name == "moveto") {
        const auto &minst = GetTypedInstr_<Script::MoveToInstr>(instr);
        MoveTo_(minst.pos, minst.duration);
    }
    else if (instr.name == "section") {
        const auto &sinst = GetTypedInstr_<Script::SectionInstr>(instr);
        if (video_writer_) {
            if (GetOptions_().report) {
                std::cout << "    Section " << video_writer_->GetChapterCount()
                          << " (" << sinst.tag << ") at "
                          << video_writer_->GetImageCount()
                          << ": " << sinst.title << "\n";
            }
            video_writer_->AddChapter(sinst.tag, sinst.title);
        }
    }
    else if (instr.name == "start") {
        is_capturing_ = true;
    }
    else if (instr.name == "tooltips") {
        const auto &tinst = GetTypedInstr_<Script::TooltipsInstr>(instr);
        TooltipFeedback::SetDelay(tinst.is_on ? 1 : 0);
    }
    else if (instr.name == "wait") {
        const auto &winst = GetTypedInstr_<Script::WaitInstr>(instr);
        MoveTo_(cursor_pos_, winst.duration);
    }

    // XXXXXXXXXXXXXXXXXXx

    // Skip snap instructions if disabled.
    if (GetOptions_().nosnap &&
        (instr.name == "snap" || instr.name == "snapobj"))
        return true;

    auto &emitter = GetEmitter();

#if XXXX
    else if (instr.name == "click") {
        //const auto &cinst = GetTypedInstr_<Script::ClickInstr>(instr);
        // XXXX emitter.AddClick(cinst.pos);
        // XXXX GetEmitter().AddClick(cursor_pos_);
    }
    else if (instr.name == "drag") {
        const auto &dinst = GetTypedInstr_<Script::DragInstr>(instr);
        emitter.AddHoverPoint(dinst.pos0);
        emitter.AddDragPoints(dinst.pos0, dinst.pos1, dinst.count);
    }
    else if (instr.name == "dragp") {
        const auto &dinst = GetTypedInstr_<Script::DragPInstr>(instr);
        emitter.AddDragPoint(dinst.phase, dinst.pos);
    }
#endif
#if XXXX
    else if (instr.name == "hover") {
        const auto &hinst = GetTypedInstr_<Script::HoverInstr>(instr);
        emitter.AddHoverPoint(hinst.pos);
    }
    else if (instr.name == "key") {
        const auto &kinst = GetTypedInstr_<Script::KeyInstr>(instr);
        emitter.AddKey(kinst.key_string);
    }
#endif
    else if (instr.name == "load") {
        const auto &linst = GetTypedInstr_<Script::LoadInstr>(instr);
        if (! LoadSession_(linst.file_name))
            return false;
    }
    else if (instr.name == "mod") {
        const auto &minst = GetTypedInstr_<Script::ModInstr>(instr);
        emitter.SetModifiedMode(minst.is_on);
    }
    else if (instr.name == "select") {
        const auto &sinst = GetTypedInstr_<Script::SelectInstr>(instr);
        Selection sel;
        BuildSelection_(sinst.names, sel);
        GetContext().selection_manager->ChangeSelection(sel);
    }
    else if (instr.name == "settings") {
        const auto &sinst = GetTypedInstr_<Script::SettingsInstr>(instr);
        const FilePath path("PublicDoc/snaps/settings/" + sinst.file_name +
                            TK::kDataFileExtension);
        if (! LoadSettings(path))
            return false;
    }
    else if (instr.name == "snap") {
        const auto &sinst = GetTypedInstr_<Script::SnapInstr>(instr);
        if (! TakeSnapshot_(sinst.rect, sinst.file_name))
            return false;
    }
    else if (instr.name == "snapobj") {
        const auto &sinst = GetTypedInstr_<Script::SnapObjInstr>(instr);
        Range2f rect;
        if (! GetNodeRect(sinst.path_string, sinst.margin, rect) ||
            ! TakeSnapshot_(rect, sinst.file_name))
            return false;
    }
    else if (instr.name == "stage") {
        const auto &sinst = GetTypedInstr_<Script::StageInstr>(instr);
        auto &stage = *GetContext().scene_context->stage;
        stage.SetScaleAndRotation(sinst.scale, sinst.angle);
    }
    else if (instr.name == "touch") {
        const auto &tinst = GetTypedInstr_<Script::TouchInstr>(instr);
        SetTouchMode_(tinst.is_on);
    }
    else if (instr.name == "view") {
        const auto &vinst = GetTypedInstr_<Script::ViewInstr>(instr);
        GetContext().scene_context->window_camera->SetOrientation(
            Rotationf::RotateInto(-Vector3f::AxisZ(), vinst.dir));
    }
    else {
        std::cerr << "--- Ignoring instruction: " + instr.name);
        return false;
    }
    else {
        std::cerr << "--- Ignoring instruction: " + instr.name);
        return false;
    }
    return true;
#endif
}

bool ScriptedApp::ProcessAction_(const Script::ActionInstr &instr) {
    ASSERTM(GetContext().action_processor->CanApplyAction(instr.action),
            Util::EnumName(instr.action));
    GetContext().action_processor->ApplyAction(instr.action);
    return true;
}

bool ScriptedApp::ProcessCaption_(const Script::CaptionInstr &instr) {
    caption_.text->SetText(instr.text);

    // Get the number of lines of text and adjust the Y scale to match.
    const size_t line_count = 1 + std::ranges::count(instr.text, '\n');
    caption_.text->SetUniformScale(line_count);

    // Scale the background to be slightly larger than the text.
    const float kTextBGFactorX = 1.1f;
    const float kTextBGFactorY = 2;
    const auto text_size = line_count * caption_.text->GetTextSize();
    caption_.bg->SetScale(Vector3f(kTextBGFactorX * text_size[0],
                                   kTextBGFactorY * text_size[1], 1));

    caption_.node->SetEnabled(true);
    caption_.pos = instr.pos;
    if (video_)
        video_->InitCaptionFade(instr.duration);
    UpdateCaption_();
    return true;
}

bool ScriptedApp::ProcessClick_(const Script::ClickInstr &instr) {
    emitter_->AddClick(cursor_pos_);
    return true;
}

bool ScriptedApp::ProcessDrag_(const Script::DragInstr &instr) {
    return false; // XXXX
}

bool ScriptedApp::ProcessFocus_(const Script::FocusInstr &instr) {
    return false; // XXXX
}

bool ScriptedApp::ProcessHand_(const Script::HandInstr &instr) {
    return false; // XXXX
}

bool ScriptedApp::ProcessHandPos_(const Script::HandPosInstr &instr) {
    return false; // XXXX
}

bool ScriptedApp::ProcessHighlight_(const Script::HighlightInstr &instr) {
    return false; // XXXX
}

bool ScriptedApp::ProcessKey_(const Script::KeyInstr &instr) {
    return false; // XXXX
}

bool ScriptedApp::ProcessLoad_(const Script::LoadInstr &instr) {
    return false; // XXXX
}

bool ScriptedApp::ProcessMoveOver_(const Script::MoveOverInstr &instr) {
    return false; // XXXX
}

bool ScriptedApp::ProcessMoveTo_(const Script::MoveToInstr &instr) {
    return false; // XXXX
}

bool ScriptedApp::ProcessSection_(const Script::SectionInstr &instr) {
    return false; // XXXX
}

bool ScriptedApp::ProcessSelect_(const Script::SelectInstr &instr) {
    return false; // XXXX
}

bool ScriptedApp::ProcessSettings_(const Script::SettingsInstr &instr) {
    return false; // XXXX
}

bool ScriptedApp::ProcessSnap_(const Script::SnapInstr &instr) {
    return false; // XXXX
}

bool ScriptedApp::ProcessSnapObj_(const Script::SnapObjInstr &instr) {
    return false; // XXXX
}

bool ScriptedApp::ProcessStage_(const Script::StageInstr &instr) {
    return false; // XXXX
}

bool ScriptedApp::ProcessState_(const Script::StateInstr &instr) {
    return false; // XXXX
}

bool ScriptedApp::ProcessStop_(const Script::StopInstr &instr) {
    return false; // XXXX
}

bool ScriptedApp::ProcessView_(const Script::ViewInstr &instr) {
    return false; // XXXX
}

bool ScriptedApp::ProcessWait_(const Script::WaitInstr &instr) {
    return false; // XXXX
}

bool ScriptedApp::PauseOrUnpause() {
    is_paused_ = ! is_paused_;
    std::cerr << "*** " << (is_paused_ ? "PAUSED" : "UNPAUSED") << "\n";
    if (is_paused_) {
        emitter_->SavePendingEvents();
        EnableMouseMotionEvents(true);
    }
    else {
        emitter_->RestorePendingEvents();
        EnableMouseMotionEvents(false);
    }
    return is_paused_;
}

bool ScriptedApp::LoadSettings(const FilePath &path) {
    const auto &context = GetContext();

    // Set the new path in the SettingsManager.
    if (! context.settings_manager->SetPath(path, false)) {
        std::cerr << "*** Unable to load settings from " << path.ToString()
                  << ": " << context.settings_manager->GetLoadError() << "\n";
        return false;
    }

    // Tell the SessionManager to update its previous path.
    context.session_manager->ChangePreviousPath(
        context.settings_manager->GetSettings().GetLastSessionPath());

    return true;
}

bool ScriptedApp::GetNodeRect(const Str &path_string, float margin,
                              Range2f &rect) {
    // Get a path to the node.
    const auto path = GetNodePath(path_string);
    if (path.empty()) {
        std::cerr << "*** No node named '" << path_string << "' found\n";
        return false;
    }

    // Compute the object-to-world matrix.
    const Matrix4f ctm = SG::CoordConv(path).GetObjectToRootMatrix();

    // For each object-coordinate bounds corner point, transform it to world
    // coordinates and project it onto the image plane to get the extents of
    // the rectangle. Note that this gives a much tighter rectangle in many
    // cases compared to transforming the bounds.
    Point3f corners[8];
    path.back()->GetBounds().GetCorners(corners);
    rect.MakeEmpty();
    const auto &frustum = *GetContext().scene_context->frustum;
    for (const auto &corner: corners)
        rect.ExtendByPoint(frustum.ProjectToImageRect(ctm * corner));

    // Add the margin.
    const Vector2f margin_vec(margin, margin);
    rect.Set(rect.GetMinPoint() - margin_vec, rect.GetMaxPoint() + margin_vec);

    // Clamp to (0,1) in both dimensions.
    rect = RangeIntersection(rect, Range2f(Point2f(0, 0), Point2f(1, 1)));
    return true;
}

SG::NodePath ScriptedApp::GetNodePath(const Str &path_string) {
    SG::NodePtr root = GetContext().scene_context->scene->GetRootNode();
    SG::NodePath path;  // Path from scene root to target object.

    // Find the node in the scene. Note that the path_string may be compound
    // ("A/B/C").
    if (path_string.contains('/')) {
        const auto parts = ion::base::SplitString(path_string, "/");
        ASSERT(parts.size() > 1U);
        for (const auto &part: parts) {
            auto sub_path = SG::FindNodePathUnderNode(root, part, true);
            if (sub_path.empty())  // Not found.
                return SG::NodePath();
            else if (path.empty())
                path = sub_path;
            else
                path = SG::NodePath::Stitch(path, sub_path);
            root = path.back();
        }
    }
    else {
        path = SG::FindNodePathUnderNode(root, path_string, true);
    }
    return path;
}

void ScriptedApp::UpdateCaption_() {
    // Set the position and face the camera. Do this every time in case the
    // view changes.
    caption_.node->TranslateTo(GetImagePlanePoint_(caption_.pos));
    caption_.node->SetRotation(GetFrustum().orientation);

    if (video_)
        video_->UpdateCaptionFade(caption_);
}

Point3f ScriptedApp::GetImagePlanePoint_(const Point2f &pos) {
    // Build a ray through the point using the view frustum.
    const Frustum frustum = GetFrustum();
    const auto ray = frustum.BuildRay(pos);

    // Intersect the ray with the plane 1 unit away along the view direction
    // and translate the fake cursor to the resulting point.
    const Vector3f view_dir = frustum.GetViewDirection();
    const Plane plane(frustum.position + view_dir, view_dir);

    float distance;
    RayPlaneIntersect(ray, plane, distance);
    return ray.GetPoint(distance);
}

Frustum ScriptedApp::GetFrustum() const {
    // Get the WindowCamera from the Scene and let it build a Frustum.
    const auto &sc = *GetContext().scene_context;
    auto cam = sc.scene->GetTypedCamera<SG::WindowCamera>();
    ASSERT(cam);
    Frustum frustum;
    cam->BuildFrustum(GetWindowSize(), frustum);
    return frustum;
}
