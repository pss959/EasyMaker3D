#include "App/ScriptedApp.h"

#include <ion/gfx/image.h>
#include <ion/image/conversionutils.h>
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
#include "Math/Frustum.h"
#include "Math/Intersection.h"
#include "Math/Linear.h"
#include "Math/Plane.h"
#include "Math/Ray.h"
#include "Models/Model.h"
#include "Models/RootModel.h"
#include "Panels/Board.h"
#include "Panels/FilePanel.h"
#include "Panels/KeyboardPanel.h"
#include "Panels/SettingsPanel.h"
#include "Parser/Parser.h"
#include "SG/CoordConv.h"
#include "SG/Gantry.h"
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

// ----------------------------------------------------------------------------
// ScriptedApp::MockFilePathList_ class.
// ----------------------------------------------------------------------------

/// Derived FilePathList class that simulates a file system so that
/// documentation images are consistent and predictable.
class ScriptedApp::MockFilePathList_ : public FilePathList {
  public:
    /// Possible settings for treating file existence.
    enum class ExistSetting {
        kTweak,  ///< Uses the file system after TweakPath().
        kFail,   ///< Always returns false.
        kPass,   ///< Always returns true.
    };

    /// Sets how subsequent calls to IsExistingFile() operate.
    void SetExistSetting(ExistSetting es) { exist_setting_ = es; }

    /// Redefines this to simulate files.
    virtual void GetContents(StrVec &subdirs, StrVec &files,
                             const Str &extension,
                             bool include_hidden) const override;
    virtual bool IsValidDirectory(const FilePath &path) const {
        const Str fn = path.GetFileName();
        return fn.empty() || fn.starts_with("Dir") ||
            fn == "stl" || fn == "maker";
    }
    virtual bool IsExistingFile(const FilePath &path) const {
        if (exist_setting_ == ExistSetting::kTweak) {
            FilePath tpath = path;
            TweakPath(tpath, true);
            return FilePathList::IsExistingFile(tpath);
        }
        else {
            return exist_setting_ == ExistSetting::kPass;
        }
    }
    virtual void TweakPath(FilePath &path, bool is_read) const override {
        // If reading from a file and tweaking is turned on, change the path to
        // a real one.
        if (is_read && exist_setting_ == ExistSetting::kTweak) {
            path = Util::ReplaceString(path.ToString(),
                                       "/projects/maker",
                                       "PublicDoc/sessions");
        }
    }

  private:
    ExistSetting exist_setting_ = ExistSetting::kTweak;
};

void ScriptedApp::MockFilePathList_::GetContents(StrVec &subdirs, StrVec &files,
                                                 const Str &extension,
                                                 bool include_hidden) const {
    // If the path is a file (has an extension), use the parent directory.
    FilePath dir = GetCurrent();
    if (dir.ToString().contains('.'))
        dir = dir.GetParentDirectory();

    // Special case for dummy paths used in doc.
    if (dir.ToString() == "/projects/maker/stl" ||
        dir.ToString() == "/projects/maker/stl/") {
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
        files.push_back("Sample" + extension);  // Needed for session tutorial.
    }
}

// ----------------------------------------------------------------------------
// ScriptedApp::CursorHandler_ class.
// ----------------------------------------------------------------------------

/// This derived Handler class is used by the ScriptedApp to update the
/// fake cursor when a mouse motion event is seen.
class ScriptedApp::CursorHandler_ : public Handler {
  public:
    using Func = std::function<void(const Point2f &)>;

    /// The constructor is passed a function to invoke when a mouse position
    /// event is handled.
    explicit CursorHandler_(const Func &func) : func_(func) {
        ASSERT(func);
    }

    /// Defines this to invoke the mouse position function.
    virtual HandleCode HandleEvent(const Event &event) override {
        if (event.flags.Has(Event::Flag::kPosition2D)) {
            func_(event.position2D);
            // Let other handlers see the event.
            return HandleCode::kHandledContinue;
        }
        return HandleCode::kNotHandled;
    }

  private:
    Func func_;
};

// ----------------------------------------------------------------------------
// ScriptedApp::PauseHandler_ class.
// ----------------------------------------------------------------------------

/// This derived Handler class is used by the ScriptedApp to handle pause
/// and unpause events.
class ScriptedApp::PauseHandler_ : public Handler {
  public:
    using Func = std::function<void()>;

    /// The constructor is passed a function to invoke when a pause key event
    /// is handled.
    explicit PauseHandler_(const Func &func) : func_(func) {
        ASSERT(func);
    }

    /// Defines this to invoke the mouse position function.
    virtual HandleCode HandleEvent(const Event &event) override {
        if (event.flags.Has(Event::Flag::kKeyPress) &&
            event.GetKeyString() == "Pause") {
            func_();
            // Let other handlers see the event.
            return HandleCode::kHandledContinue;
        }
        return HandleCode::kNotHandled;
    }

  private:
    Func func_;
};

// ----------------------------------------------------------------------------
// ScriptedApp::Video_ class.
// ----------------------------------------------------------------------------

class ScriptedApp::Video_ {
  public:
    Video_(const FilePath &script_path, const Vector2ui &size,
           const Str &format, const int fps, bool do_capture);

    /// Enables video capturing.
    void EnableCapturing(bool is_on) { is_capturing_ = is_on; }

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

    /// Updates fade-in/out for the highlight rectangles. Returns false if
    /// highlights have completely faded out.
    bool UpdateHighlightFade(std::vector<SG::NodePtr> &highlights);

    /// Adds a chapter with the given tag and title at the current frame in the
    /// video.
    void AddChapter(const Str &tag, const Str &title) {
        if (video_writer_)
            video_writer_->AddChapter(tag, title);
    }

    /// Captures a frame using the given renderer and image size.
    void CaptureFrame(IRenderer &renderer, const Vector2ui &size);

    /// Writes the captured video frames to the file.
    void WriteToFile() {
        if (video_writer_)
            video_writer_->WriteToFile();
    }

  private:
    /// This struct is used to fade items (such as captions or the highlight
    /// rectangle) in and out over a duration.
    struct FadeData_ {
        float duration = 0;   ///< Fade-in/out duration in seconds.
        float elapsed  = 0;   ///< Elapsed time in seconds.
    };

    const int fps_;  ///< Frames per second passed to constructor.

    /// Creates and writes a video to a file.
    std::unique_ptr<VideoWriter> video_writer_;

    /// This is set to true when a start instruction is processed.
    bool is_capturing_ = false;

    FadeData_ caption_fade_data_;    ///< Caption fading/visibility.
    FadeData_ highlight_fade_data_;  ///< Highlight rectangle fading/visibility.

    /// Updates fade-in/out using \p fade_data. Returns the alpha.
    float UpdateFade_(FadeData_ &fade_data);
};

ScriptedApp::Video_::Video_(const FilePath &script_path, const Vector2ui &size,
                            const Str &format, int fps,
                            bool do_capture) : fps_(fps) {
    if (do_capture) {
        const VideoWriter::Format vformat =
            format == "rgbmp4" ? VideoWriter::Format::kRGBMP4 :
            format == "yuvmp4" ? VideoWriter::Format::kYUVMP4 :
            VideoWriter::Format::kWEBM;
        video_writer_.reset(new VideoWriter(vformat));

        // Set up the output path.
        FilePath video_path = FilePath::Join("PublicDoc/docs/extra/videos/",
                                             script_path);
        video_path.ReplaceExtension("." + video_writer_->GetExtension());
        video_writer_->Init(video_path, size, fps);
    }
}

void ScriptedApp::Video_::UpdateCaptionFade(const Caption_ &caption) {
    // Note that this fades just the text, not the background.
    if (caption_fade_data_.elapsed <= caption_fade_data_.duration) {
        const float alpha = UpdateFade_(caption_fade_data_);
        Color c = caption.text->GetColor();
        c[3] = alpha;
        caption.text->SetTextColor(c);
    }
    else {
        caption.node->SetEnabled(false);  // Fade has finished.
    }
}

bool ScriptedApp::Video_::UpdateHighlightFade(
    std::vector<SG::NodePtr> &highlights) {
    if (highlight_fade_data_.elapsed <= highlight_fade_data_.duration) {
        const float alpha = UpdateFade_(highlight_fade_data_);
        for (auto hl: highlights) {
            Color c = hl->GetBaseColor();
            c[3] = alpha;
            hl->SetBaseColor(c);
        }
        return true;
    }
    return false;  // Fade has finished.
}

void ScriptedApp::Video_::CaptureFrame(IRenderer &renderer,
                                       const Vector2ui &size) {
    if (video_writer_ && is_capturing_) {
        const auto image =
            renderer.ReadImage(Viewport::BuildWithSize(Point2ui(0, 0), size));

        // Rows of image need to be inverted (GL vs stblib).
        ion::image::FlipImage(image);
        video_writer_->AddImage(*image);
    }
}

float ScriptedApp::Video_::UpdateFade_(FadeData_ &fade_data) {
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
        alpha = 0;

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
    REG_FUNC_(DragStart);
    REG_FUNC_(DragEnd);
    REG_FUNC_(Files);
    REG_FUNC_(Focus);
    REG_FUNC_(HandModel);
    REG_FUNC_(HandMove);
    REG_FUNC_(HandPoint);
    REG_FUNC_(HandPos);
    REG_FUNC_(HandTurn);
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

    options_.display_vr    = true;  // Allow controllers to be shown.
    options_.maximize      = args.GetBool("--maximize");
    options_.remain        = args.GetBool("--remain");
    options_.report        = args.GetBool("--report");
    options_.offscreen     = args.GetBool("--offscreen");
    options_.show_session_panel = false;

    // Uncomment this if needed to help with debugging.
    // options_.do_ion_remote = true;

    // Window size. Note that this must have the same aspect ratio as a
    // maximized window.
    int size_n = args.HasArg("--size") ? args.GetAsInt("--size", 1) : 1;
    if (size_n <= 0)
        size_n = 1;
    options_.window_size.Set(1024 / size_n, 552 / size_n);
}

bool ScriptedApp::ProcessScript(const FilePath &script_dir,
                                const FilePath &script_path, bool do_video) {
    // Read the script first.
    if (! script_.ReadScript(FilePath::Join(script_dir, script_path)))
        return false;

    // Initialize the application.
    if (! Init_())
        return false;

    // Initialize video if requested.
    if (do_video)
        video_.reset(new Video_(script_path, GetWindowSize(),
                                options_.vidformat, options_.fps,
                                ! options_.dryrun));

    // Run the Application main loop, which calls ProcessFrame().
    try {
        MainLoop();
    }
    catch (AssertException &ex) {
        std::cerr << "*** Caught Assertion failure:\n" << ex.what() << "\n";
        std::cerr << "*** STACK:\n";
        for (const auto &s: ex.GetStackTrace())
            std::cerr << "  " << s << "\n";
        return false;
    }
    catch (std::exception &ex) {
        std::cerr << "*** Caught exception: " << ex.what() << "\n";
        return false;
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
    bool keep_going = true;
    bool was_stopped = false;

    if (are_more_instructions || events_pending) {
        UpdateCaption_();
        UpdateHighlights_();
    }

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
        // Do nothing; just keep going.
    }

    else if (are_more_instructions) {
        const auto &instr = *script_.GetInstructions()[cur_instruction_];
        const size_t instr_count = script_.GetInstructions().size();
        if (options_.report)
            std::cout << "  Processing " << instr.name
                      << " (instruction " << (cur_instruction_ + 1)
                      << " of " << instr_count << ") on line "
                      << instr.line_number << " at frame " << render_count
                      << "\n";
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
        keep_going  = false;
        was_stopped = true;
    }

    // Capture the current frame if necessary.
    if (video_)
        video_->CaptureFrame(GetRenderer(), GetWindowSize());

    // If processing is done, finish up.
    if (was_stopped) {
        Finish_();
        keep_going = options_.remain;
    }

    return keep_going;
}

bool ScriptedApp::IsInModifiedMode() const {
    return emitter_->IsInModifiedMode();
}

bool ScriptedApp::IsAnimationEnabled() const {
    // The "animation" setting is used to control this as well.
    return Model::IsPlacementAnimationEnabled();
}

// ----------------------------------------------------------------------------
// Initialization.
// ----------------------------------------------------------------------------

bool ScriptedApp::Init_() {
    if (! Application::Init(options_))
        return false;

    emitter_.reset(new ScriptEmitter);
    AddEmitter(emitter_);

    const auto &context = GetContext();
    ASSERT(context.scene_context);
    ASSERT(context.scene_context->scene);

    // Use default settings file so that state is deterministic.
    const FilePath path("PublicDoc/settings/Settings" + TK::kDataFileExtension);
    if (! LoadSettings_(path))
        return false;

    InitControllers_();
    InitMockFilePathList_();
    InitScene_();
    InitHandlers_();

#if ENABLE_DEBUG_FEATURES
    // Make sure there is no debug text visible.
    Debug::DisplayDebugText("");
#endif

    // Ignore mouse events from GLFWViewer so they do not interfere with the
    // click and drag events.
    EnableMouseMotionEvents(false);

    // No need to ask before quitting a scripted app.
    SetAskBeforeQuitting(false);

    // Use a constant time increment per frame for animation so the animations
    // are not subject to inconsistent frame times due to capture.
    context.animation_manager->SetFrameIncrement(1.f / options_.fps);

    // Set a huge duration (20 seconds) so long presses do not occur. Video
    // capture is sometimes slow enough to make a press seem very long.
    SetLongPressDuration(20);

    // Disable tooltips by default - they can appear if frame grabbing is very
    // slow.
    SetTooltipDelay(0);

    // Fake export from the SessionManager, since the path is likely bogus.
    context.session_manager->SetFakeExport(true);

    return true;
}

void ScriptedApp::InitControllers_() {
    const auto &sc = *GetContext().scene_context;

    // Turn off controllers until they are specifically added.
    sc.left_controller->SetEnabled(false);
    sc.right_controller->SetEnabled(false);

    // Set render offsets for the controllers.
    SetControllerRenderOffsets(-ScriptEmitter::kLeftControllerOffset,
                               -ScriptEmitter::kRightControllerOffset);
}

void ScriptedApp::InitMockFilePathList_() {
    // Use the MockFilePathList_ for the FilePanel, ImportToolPanel, and
    // SettingsPanel.
    mock_fpl_.reset(new MockFilePathList_);

    const auto &panel_mgr = *GetContext().panel_manager;
    auto fp = panel_mgr.GetTypedPanel<FilePanel>("FilePanel");
    auto ip = panel_mgr.GetTypedPanel<FilePanel>("ImportToolPanel");
    auto sp = panel_mgr.GetTypedPanel<SettingsPanel>("SettingsPanel");
    fp->SetFilePathList(mock_fpl_.get());
    ip->SetFilePathList(mock_fpl_.get());
    sp->SetFilePathList(mock_fpl_.get());
}

void ScriptedApp::InitScene_() {
    const auto &sc = *GetContext().scene_context;
    auto room = SG::FindNodeInScene(*sc.scene, "Room");

    // Parse the Scripting scene data and add it to the Room.
    Parser::Parser parser;
    auto scripting_root = std::dynamic_pointer_cast<SG::Node>(
        parser.ParseFile(FilePath::GetResourcePath("nodes", "Scripting.emd")));
    ASSERT(scripting_root);
    room->AddChild(scripting_root);

    // Access the FakeCursor and position it just in front of the camera.
    cursor_ = SG::FindNodeUnderNode(*scripting_root, "FakeCursor");
    MoveFakeCursorTo_(Point2f(.5f, .5f));  // In the middle.

    // Access the highlight parent and node.
    highlight_parent_ = SG::FindNodeUnderNode(*scripting_root, "Highlights");
    highlight_node_   = SG::FindNodeUnderNode(*scripting_root, "HighlightRect");

    // Access the caption nodes.
    caption_.node = SG::FindNodeUnderNode(*scripting_root, "Caption");
    caption_.bg   = SG::FindNodeUnderNode(*caption_.node,  "Background");
    caption_.text = SG::FindTypedNodeUnderNode<SG::TextNode>(*caption_.node,
                                                            "Text");

    // When the height slider is dragged, anything positioned relative to the
    // camera needs to be updated.
    sc.height_slider->GetValueChanged().AddObserver(
        this, [&](Widget &, const float &val){
            MoveFakeCursorTo_(cursor_pos_);
            UpdateCaption_();
        });

    // Save the default gantry height.
    default_gantry_height_ = GetContext().scene_context->gantry->GetHeight();
}

void ScriptedApp::InitHandlers_() {
    // Add a Handler_ to update the fake cursor when the mouse is moved. Insert
    // it at the beginning so no other handler steals the event.
    cursor_handler_.reset(
        new CursorHandler_([&](const Point2f &p){ MoveFakeCursorTo_(p); }));
    GetContext().event_manager->InsertHandler(cursor_handler_);

    // Add a Handler_ to allow pausing when the "remain" option is specified.
    // Insert it at the beginning so no other handler steals the event.
    if (options_.remain) {
        auto pause_func = [&](){
            const bool is_paused = PauseOrUnpause_();
            cursor_handler_->SetEnabled(! is_paused);
        };
        pause_handler_.reset(new PauseHandler_(pause_func));
        GetContext().event_manager->InsertHandler(pause_handler_);
    }
}

// ----------------------------------------------------------------------------
// Instruction processing.
// ----------------------------------------------------------------------------

bool ScriptedApp::ProcessInstruction_(const Script::Instr &instr) {
    const auto it = func_map_.find(instr.name);
    ASSERTM(it != func_map_.end(), "Bad instruction name: " + instr.name);
    return it->second(instr);
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
    if (instr.device == Event::Device::kMouse) {
        emitter_->AddMouseButton(Event::Button::kMouse1, true,  cursor_pos_);
        emitter_->AddMouseButton(Event::Button::kMouse1, false, cursor_pos_);
    }
    else {
        const Hand hand = instr.device == Event::Device::kLeftController ?
            Hand::kLeft : Hand::kRight;
        const int index = Util::EnumInt(hand);
        emitter_->AddControllerButton(hand, instr.button, true,
                                      controller_pos_[index],
                                      controller_rot_[index]);
        emitter_->AddControllerButton(hand, instr.button, false,
                                      controller_pos_[index],
                                      controller_rot_[index]);
    }
    return true;
}

bool ScriptedApp::ProcessDrag_(const Script::DragInstr &instr) {
    const Point2f end_pos = cursor_pos_ + instr.motion;
    const size_t  frames  = GetFrameCount_(instr.duration);

    emitter_->AddMouseButton(instr.button, true,  cursor_pos_);
    emitter_->AddMouseMotion(cursor_pos_, end_pos, frames);
    emitter_->AddMouseButton(instr.button, false, end_pos);

    return true;
}

bool ScriptedApp::ProcessDragStart_(const Script::DragStartInstr &instr) {
    // Start drag if necessary.
    if (! emitter_->IsDragging()) {
        drag_start_pos_ = cursor_pos_;
        emitter_->AddMouseButton(Event::Button::kMouse1, true, cursor_pos_);
    }
    const Point2f end_pos = drag_start_pos_ + instr.motion;
    const size_t  frames  = GetFrameCount_(instr.duration);
    emitter_->AddMouseMotion(cursor_pos_, end_pos, frames);
    MoveFakeCursorTo_(end_pos);
    return true;
}

bool ScriptedApp::ProcessDragEnd_(const Script::DragEndInstr &instr) {
    emitter_->AddMouseButton(Event::Button::kMouse1, false, cursor_pos_);
    return true;
}

bool ScriptedApp::ProcessFiles_(const Script::FilesInstr &instr) {
    mock_fpl_->SetExistSetting(
        instr.setting == "fail" ? MockFilePathList_::ExistSetting::kFail :
        instr.setting == "pass" ? MockFilePathList_::ExistSetting::kPass :
        MockFilePathList_::ExistSetting::kTweak);
    return true;
}

bool ScriptedApp::ProcessFocus_(const Script::FocusInstr &instr) {
    // Access the current Board.
    auto board = GetContext().board_manager->GetCurrentBoard();
    if (! board) {
        std::cerr << "*** No current board for focusing pane\n";
        return false;
    }
    auto panel = board->GetCurrentPanel();
    if (! panel) {
        std::cerr << "*** No current panel for focusing pane\n";
        return false;
    }
    auto pane = panel->GetPane()->FindSubPane(instr.pane_name);
    if (! pane || ! pane->GetInteractor()) {
        std::cerr << "*** Could not find interactive pane '" << instr.pane_name
                  << "' in current board\n";
        return false;
    }
    panel->SetFocusedPane(pane);
    return true;
}

bool ScriptedApp::ProcessHandModel_(const Script::HandModelInstr &instr) {
    const auto &sc = *GetContext().scene_context;
    auto &controller = instr.hand == Hand::kLeft ?
        *sc.left_controller : *sc.right_controller;

    if (instr.controller == "None") {
        controller.SetEnabled(false);
        return true;
    }

    const Str file_start = "models/controllers/" + instr.controller +
        "_" + Util::EnumToWord(instr.hand);

    const Str mesh_file = file_start + ".tri";
    const Str tex_file  = file_start + ".jpg";

    Str mesh_data;
    Controller::CustomModel cm;
    if (! Util::ReadFile(mesh_file, mesh_data) ||
        ! cm.mesh.FromBinaryString(mesh_data)) {
        std::cerr << "*** Error reading controller mesh data from "
                  << mesh_file << "\n";
        return false;
    }

    cm.texture_image = Util::ReadImage(tex_file, false);
    if (! cm.texture_image) {
        std::cerr << "*** Error reading controller texture image from "
                  << tex_file << "\n";
        return false;
    }

    controller.UseCustomModel(cm);
    controller.SetEnabled(true);
    controller.SetGripGuideType(GripGuideType::kBasic);
    controller.ShowAll(true);
    controller.ShowGripHover(false, Point3f::Zero(), Color::White());

    // Since at least one controller is in use, turn off the RadialMenu parent
    // in the room, since the menus will be attached to the controllers.
    const auto parent = SG::FindNodeInScene(*sc.scene, "RadialMenus");
    parent->SetEnabled(false);

    return true;
}

bool ScriptedApp::ProcessHandMove_(const Script::HandMoveInstr &instr) {
    const int     index   = Util::EnumInt(instr.hand);
    const size_t  frames  = GetFrameCount_(instr.duration);
    const Point3f end_pos = controller_pos_[index] + instr.trans;
    const auto    &rot    = controller_rot_[index];

    // Start drag if button was specified.
    if (instr.button != Event::Button::kNone) {
        emitter_->AddControllerButton(instr.hand, instr.button, true,
                                      controller_pos_[index], rot);
    }

    emitter_->AddControllerMotion(instr.hand, controller_pos_[index], end_pos,
                                  rot, rot, frames);

    // End drag if button was specified.
    if (instr.button != Event::Button::kNone) {
        emitter_->AddControllerButton(instr.hand, instr.button, false,
                                      end_pos, rot);
    }

    controller_pos_[index] = end_pos;
    UpdateGripHover_();
    return true;
}

bool ScriptedApp::ProcessHandPoint_(const Script::HandPointInstr &instr) {
    const int index = Util::EnumInt(instr.hand);
    const size_t frames = GetFrameCount_(instr.duration);

    // Get the actual position of the controller in world coordinates.
    const Point3f &pos = controller_pos_[index];
    const Point3f world_pos =
        ScriptEmitter::GetWorldControllerPos(instr.hand, pos);

    // Get the center of the target Node in world coordinates.
    SG::NodePath path = GetNodePath_(instr.path_string);
    if (path.empty()) {
        std::cerr << "*** No node named '" << instr.path_string << "' found\n";
        return false;
    }
    const auto center = SG::CoordConv(path).ObjectToRoot(Point3f::Zero());

    // Rotate the controller to point at the Node.
    const auto dir = ion::math::Normalized(center - world_pos);
    const Rotationf end_rot = Rotationf::RotateInto(-Vector3f::AxisZ(), dir);

    emitter_->AddControllerMotion(instr.hand, pos, pos,
                                  controller_rot_[index], end_rot, frames);

    controller_rot_[index] = end_rot;
    UpdateGripHover_();
    return true;
}

bool ScriptedApp::ProcessHandPos_(const Script::HandPosInstr &instr) {
    emitter_->AddControllerMotion(instr.hand, instr.pos, instr.pos,
                                  instr.rot, instr.rot, 1);

    const int index = Util::EnumInt(instr.hand);
    controller_pos_[index] = instr.pos;
    controller_rot_[index] = instr.rot;
    return true;
}

bool ScriptedApp::ProcessHandTurn_(const Script::HandTurnInstr &instr) {
    const int       index   = Util::EnumInt(instr.hand);
    const size_t    frames  = GetFrameCount_(instr.duration);
    const Rotationf end_rot = controller_rot_[index] * instr.rot;
    const auto      &pos    = controller_pos_[index];

    // Start drag if button was specified.
    if (instr.button != Event::Button::kNone) {
        emitter_->AddControllerButton(instr.hand, instr.button, true,
                                      pos, controller_rot_[index]);
    }

    emitter_->AddControllerMotion(instr.hand, pos, pos,
                                  controller_rot_[index], end_rot, frames);

    // End drag if button was specified.
    if (instr.button != Event::Button::kNone) {
        emitter_->AddControllerButton(instr.hand, instr.button, false,
                                      pos, end_rot);
    }

    controller_rot_[index] = end_rot;
    UpdateGripHover_();
    return true;
}

bool ScriptedApp::ProcessHighlight_(const Script::HighlightInstr &instr) {
    // Set up rectangles for all nodes to highlight.
    for (const auto &path: instr.path_strings) {
        Range2f rect;
        if (! GetNodeRect_(path, instr.margin, rect))
            return false;
        auto highlight = CreateHighlight_(rect);
        highlights_.push_back(highlight);
        highlight_parent_->AddChild(highlight);
    }
    if (video_) {
        video_->InitHighlightFade(instr.duration);
        UpdateHighlights_();
    }
    return true;
}

bool ScriptedApp::ProcessKey_(const Script::KeyInstr &instr) {
    emitter_->AddKey(instr.key_string);
    return true;
}

bool ScriptedApp::ProcessLoad_(const Script::LoadInstr &instr) {
    // Empty file name means start a new session.
    if (instr.file_name.empty()) {
        GetContext().session_manager->NewSession();
        if (options_.report)
            std::cout << "    Started new session\n";
    }
    else {
        const FilePath path("PublicDoc/sessions/" + instr.file_name +
                            TK::kSessionFileExtension);
        Str error;
        if (! GetContext().session_manager->LoadSession(path, error)) {
            std::cerr << "*** Error loading session from '" << path
                      << "':" << error << "\n";
            return false;
        }
        if (options_.report)
            std::cout << "    Loaded session from '" << path << "'\n";
    }
    return true;
}

bool ScriptedApp::ProcessMoveOver_(const Script::MoveOverInstr &instr) {
    // Get the path from the scene root to target object.
    SG::NodePath path = GetNodePath_(instr.path_string);
    if (path.empty()) {
        std::cerr << "*** No node named '" << instr.path_string << "' found\n";
        return false;
    }

    // Project the center of the object in world coordinates onto the Frustum
    // image plane to get the point to move to.
    const auto center = SG::CoordConv(path).ObjectToRoot(Point3f::Zero());
    MoveTo_(GetFrustum_().ProjectToImageRect(center) + instr.offset,
            instr.duration);
    return true;
}

bool ScriptedApp::ProcessMoveTo_(const Script::MoveToInstr &instr) {
    MoveTo_(instr.pos, instr.duration);
    return true;
}

bool ScriptedApp::ProcessSection_(const Script::SectionInstr &instr) {
    if (video_)
        video_->AddChapter(instr.tag, instr.title);
    return true;
}

bool ScriptedApp::ProcessSelect_(const Script::SelectInstr &instr) {
    Selection sel;
    const auto &root_model = GetContext().scene_context->root_model;
    for (const auto &name: instr.names) {
        SelPath path(SG::FindNodePathUnderNode(root_model, name, false));
        sel.Add(path);
    }
    GetContext().selection_manager->ChangeSelection(sel);
    return true;
}

bool ScriptedApp::ProcessSettings_(const Script::SettingsInstr &instr) {
    const FilePath path("PublicDoc/settings/" + instr.file_name +
                        TK::kDataFileExtension);
    return LoadSettings_(path);
}

bool ScriptedApp::ProcessSnap_(const Script::SnapInstr &instr) {
    return options_.dryrun ? true : TakeSnapshot_(instr.rect, instr.file_name);
}

bool ScriptedApp::ProcessSnapObj_(const Script::SnapObjInstr &instr) {
    if (options_.dryrun) {
        return true;
    }
    else {
        Range2f rect;
        return GetNodeRect_(instr.path_string, instr.margin, rect) &&
            TakeSnapshot_(rect, instr.file_name);
    }
}

bool ScriptedApp::ProcessStage_(const Script::StageInstr &instr) {
    auto &stage = *GetContext().scene_context->stage;
    stage.SetScaleAndRotation(instr.scale, instr.angle);
    return true;
}

bool ScriptedApp::ProcessState_(const Script::StateInstr &instr) {
    if (instr.setting == "animation") {
        Model::EnablePlacementAnimation(instr.is_on);
    }
    else if (instr.setting == "cursor") {
        cursor_->SetEnabled(instr.is_on);
    }
    else if (instr.setting == "headset") {
        emitter_->AddHeadsetButton(instr.is_on);
    }
    else if (instr.setting == "mod") {
        emitter_->SetModifiedMode(instr.is_on);
    }
    else if (instr.setting == "tooltips") {
        SetTooltipDelay(instr.is_on ? 1 : 0);
    }
    else if (instr.setting == "touch") {
        SetTouchMode_(instr.is_on);
    }
    else if (instr.setting == "video") {
        if (video_)
            video_->EnableCapturing(instr.is_on);
    }
    else {
        ASSERTM(false, "Bad state setting name: " + instr.setting);
        return false;
    }
    return true;
}

bool ScriptedApp::ProcessStop_(const Script::StopInstr &instr) {
    // This is handled elsewhere.
    return true;
}

bool ScriptedApp::ProcessView_(const Script::ViewInstr &instr) {
    const auto &sc = *GetContext().scene_context;
    auto &gantry = *sc.gantry;
    auto &cam    = *sc.window_camera;
    // A zero-length direction indicates that the view should be reset.
    if (instr.dir == Vector3f::Zero()) {
        gantry.SetHeight(default_gantry_height_);
        cam.SetOrientation(Rotationf::Identity());
    }
    else {
        gantry.SetHeight(instr.gantry_height);
        cam.SetOrientation(Rotationf::RotateInto(-Vector3f::AxisZ(),
                                                 instr.dir));
    }
    return true;
}

bool ScriptedApp::ProcessWait_(const Script::WaitInstr &instr) {
    MoveTo_(cursor_pos_, instr.duration);
    return true;
}

// ----------------------------------------------------------------------------
// Other functions.
// ----------------------------------------------------------------------------

void ScriptedApp::Finish_() {
    // Disable the handlers so the fake cursor does not move any more (and
    // there is no reason to pause).
    cursor_handler_->SetEnabled(false);
    if (pause_handler_)
        pause_handler_->SetEnabled(false);

    // Write the resulting video if enabled.
    if (video_)
        video_->WriteToFile();

    // Clear the Video_ pointer so this does not happen again in case Finish_()
    // is called again (if remain flag is set).
    video_.reset();

    EnableMouseMotionEvents(true);
}

Frustum ScriptedApp::GetFrustum_() const {
    // Get the WindowCamera from the Scene and let it build a Frustum.
    const auto &sc = *GetContext().scene_context;
    auto cam = sc.scene->GetTypedCamera<SG::WindowCamera>();
    ASSERT(cam);
    Frustum frustum;
    cam->BuildFrustum(GetWindowSize(), frustum);
    return frustum;
}

Point3f ScriptedApp::GetImagePlanePoint_(const Point2f &pos) {
    // Build a ray through the point using the view frustum.
    const Frustum frustum = GetFrustum_();
    const auto ray = frustum.BuildRay(pos);

    // Intersect the ray with the plane 1 unit away along the view direction
    // and translate the fake cursor to the resulting point.
    const Vector3f view_dir = frustum.GetViewDirection();
    const Plane plane(frustum.position + view_dir, view_dir);

    float distance;
    RayPlaneIntersect(ray, plane, distance);
    return ray.GetPoint(distance);
}

SG::NodePath ScriptedApp::GetNodePath_(const Str &path_string) {
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

bool ScriptedApp::GetNodeRect_(const Str &path_string, float margin,
                               Range2f &rect) {
    // Get a path to the node.
    const auto path = GetNodePath_(path_string);
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

SG::NodePtr ScriptedApp::CreateHighlight_(const Range2f &rect) {
    // Clone the highlight node.
    auto highlight = highlight_node_->CloneTyped<SG::Node>(true);

    // The highlight is created as a frame composed of 4 rectangles. The top
    // and bottom rectangles run the full width and the left/right rectangles
    // fit between them.
    auto t = SG::FindNodeUnderNode(*highlight, "Top");
    auto b = SG::FindNodeUnderNode(*highlight, "Bottom");
    auto l = SG::FindNodeUnderNode(*highlight, "Left");
    auto r = SG::FindNodeUnderNode(*highlight, "Right");

    // The highlight rectangle is in the image plane, so choose a width for
    // each piece that works.
    const float kWidth = .01f;

    const auto min    = GetImagePlanePoint_(rect.GetMinPoint());
    const auto max    = GetImagePlanePoint_(rect.GetMaxPoint());
    const auto size   = max - min;
    const auto center = .5f * (min + max);
    const Vector3f y_off(0, .5f * size[1], 0);
    const Vector3f x_off(.5f * size[0], 0, 0);

    t->SetScale(Vector3f(size[0] + kWidth, kWidth, 1));
    b->SetScale(Vector3f(size[0] + kWidth, kWidth, 1));
    l->SetScale(Vector3f(kWidth, size[1] - kWidth, 1));
    r->SetScale(Vector3f(kWidth, size[1] - kWidth, 1));

    t->TranslateTo(center + y_off);
    b->TranslateTo(center - y_off);
    l->TranslateTo(center - x_off);
    r->TranslateTo(center + x_off);

    highlight->SetEnabled(true);

    return highlight;
}

bool ScriptedApp::LoadSettings_(const FilePath &path) {
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

void ScriptedApp::MoveFakeCursorTo_(const Point2f &pos) {
    cursor_->TranslateTo(GetImagePlanePoint_(pos));
    cursor_pos_ = pos;
}

void ScriptedApp::MoveTo_(const Point2f &pos, float duration) {
    const size_t frames = GetFrameCount_(duration);
    emitter_->AddMouseMotion(cursor_pos_, pos, frames);
}

bool ScriptedApp::PauseOrUnpause_() {
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

void ScriptedApp::SetTouchMode_(bool is_on) {
    ForceTouchMode(is_on);

    // Tell the Boards.
    auto &sc = *GetContext().scene_context;
    const Point3f cam_pos = is_on ?
        sc.vr_camera->GetCurrentPosition() : Point3f::Zero();
    sc.app_board->SetUpForTouch(cam_pos);
    sc.tool_board->SetUpForTouch(cam_pos);
    sc.key_board->SetUpForTouch(cam_pos);
    ASSERT(sc.keyboard_panel);
    if (! sc.key_board->GetCurrentPanel())
        sc.key_board->SetPanel(sc.keyboard_panel);

    // Set controller state.
    sc.left_controller->SetTouchMode(is_on);
    sc.right_controller->SetTouchMode(is_on);
}

bool ScriptedApp::TakeSnapshot_(const Range2f &rect, const Str &file_name) {
    const auto &minp = rect.GetMinPoint();
    const auto  size = rect.GetSize();

    const auto window_size = GetWindowSize();
    const int x = static_cast<int>(minp[0] * window_size[0]);
    const int y = static_cast<int>(minp[1] * window_size[1]);
    const int w = static_cast<int>(size[0] * window_size[0]);
    const int h = static_cast<int>(size[1] * window_size[1]);

    const auto recti = Viewport::BuildWithSize(Point2ui(x, y), Vector2ui(w, h));
    const auto image = GetRenderer().ReadImage(recti);
    // Rows of image need to be inverted (GL vs stblib).
    ion::image::FlipImage(image);

    const FilePath path("PublicDoc/docs/images/" + file_name);
    if (! Util::WriteImage(path, *image, false)) {
        std::cerr << "*** Error saving snap image to = '"
                  << path.ToString() << "'\n";
        return false;
    }
    std::cout << "    Saved snap image to = '" << path.ToString() << "'\n";
    return true;
}

void ScriptedApp::UpdateCaption_() {
    // Set the position and face the camera. Do this every time in case the
    // view changes.
    caption_.node->TranslateTo(GetImagePlanePoint_(caption_.pos));
    caption_.node->SetRotation(GetFrustum_().orientation);

    if (video_)
        video_->UpdateCaptionFade(caption_);
}

void ScriptedApp::UpdateHighlights_() {
    if (video_ && ! video_->UpdateHighlightFade(highlights_))
        highlight_parent_->ClearChildren();
}

void ScriptedApp::UpdateGripHover_() {
    // Emit a position/orientation event for each controller so that grip hover
    // is activated.
    for (auto hand: Util::EnumValues<Hand>()) {
        const int  index = Util::EnumInt(hand);
        const auto &pos  = controller_pos_[index];
        const auto &rot  = controller_rot_[index];
        emitter_->AddControllerMotion(hand, pos, pos, rot, rot, 1);
    }
}

size_t ScriptedApp::GetFrameCount_(float duration) const {
    // Return at least 1 frame.
    return std::max(1.f, duration * options_.fps);
}
