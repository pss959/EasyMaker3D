#include <deque>
#include <string>
#include <vector>

#include <ion/gfx/image.h>
#include <ion/image/conversionutils.h>

#include "App/Application.h"
#include "App/Args.h"
#include "App/Renderer.h"
#include "App/SceneContext.h"
#include "App/Selection.h"
#include "App/SnapScript.h"
#include "Base/IEmitter.h"
#include "Base/Tuning.h"
#include "Debug/Shortcuts.h"
#include "Handlers/Handler.h"
#include "Items/Board.h"
#include "Items/Controller.h"
#include "Items/Settings.h"
#include "Managers/ActionManager.h"
#include "Managers/CommandManager.h"
#include "Managers/PanelManager.h"
#include "Managers/SelectionManager.h"
#include "Managers/SessionManager.h"
#include "Managers/SettingsManager.h"
#include "Math/Types.h"
#include "Models/RootModel.h"
#include "Panels/FilePanel.h"
#include "Panels/KeyboardPanel.h"
#include "SG/Search.h"
#include "SG/VRCamera.h"
#include "SG/WindowCamera.h"
#include "Tests/TestContext.h"
#include "Util/Assert.h"
#include "Util/Delay.h"
#include "Util/FilePath.h"
#include "Util/FilePathList.h"
#include "Util/KLog.h"
#include "Util/Read.h"
#include "Util/Write.h"
#include "Widgets/StageWidget.h"

// ----------------------------------------------------------------------------
// Constants.
// ----------------------------------------------------------------------------

/// Default rest position for the left controller.
static Point3f kLeftControllerPos{-.18f, 14.06, 59.5f};

/// Default rest position for the right controller.
static Point3f kRightControllerPos{.18f, 14.06, 59.5f};

/// Offset to add to left controller position for event position.
static Vector3f kLeftControllerOffset{0, -.12f, 0};

/// Offset to add to right controller position for event position.
static Vector3f kRightControllerOffset{0, .12f, 0};

// ----------------------------------------------------------------------------
// Emitter_ class.
// ----------------------------------------------------------------------------

/// Emitter_ is a derived IEmitter class used to create events to simulate
/// mouse clicks, mouse drags and key presses.
class Emitter_ : public IEmitter {
  public:
    using DIPhase    = SnapScript::DragInstr::Phase;
    using KModifiers = Util::Flags<Event::ModifierKey>;

    /// Sets modified mode for subsequent clicks and drags. It is off by
    /// default.
    void SetModifiedMode(bool is_on) { is_mod_ = is_on; }

    /// Adds a click to emit.
    void AddClick(const Point2f &pos);

    /// Adds an event to simulate a mouse hover at a given position.
    void AddHoverPoint(const Point2f &pos);

    /// Adds a drag point to emit.
    void AddDragPoint(DIPhase phase, const Point2f &pos);

    /// Adds a key press/release to simulate.
    void AddKey(const std::string &key, const KModifiers &modifiers);

    /// Adds a controller position.
    void AddControllerPos(Hand hand, const Point3f &pos, const Rotationf &rot);

    /// Adds a VR headset button press or release.
    void AddHeadsetButton(bool is_press);

    /// Returns true if there are events left to process.
    bool HasPendingEvents() const { return ! events_.empty(); }

    virtual void EmitEvents(std::vector<Event> &events) override;
    virtual void FlushPendingEvents() {}

  private:
    /// Whether modified mode is on.
    bool               is_mod_ = false;

    /// Events left to emit.
    std::deque<Event>  events_;

    /// Set to true if the previous event was a button press. This is used to
    /// detect clicks to handle timeout correctly.
    bool               prev_was_button_press_ = false;

    /// Set to true while waiting for a click to be processed after a timeout.
    bool               waited_for_click_ = false;
};

void Emitter_::AddClick(const Point2f &pos) {
    Event event;
    event.is_modified_mode = is_mod_;
    event.device           = Event::Device::kMouse;
    event.button           = Event::Button::kMouse1;
    event.position2D       = pos;
    event.flags.Set(Event::Flag::kPosition2D);

    // Press.
    event.flags.Set(Event::Flag::kButtonPress);
    events_.push_back(event);

    // Release.
    event.flags.Reset(Event::Flag::kButtonPress);
    event.flags.Set(Event::Flag::kButtonRelease);
    events_.push_back(event);
}

void Emitter_::AddHoverPoint(const Point2f &pos) {
    Event event;
    event.is_modified_mode = is_mod_;
    event.device           = Event::Device::kMouse;
    event.position2D       = pos;
    event.flags.Set(Event::Flag::kPosition2D);
    events_.push_back(event);
}

void Emitter_::AddDragPoint(DIPhase phase, const Point2f &pos) {
    Event event;
    event.is_modified_mode = is_mod_;
    event.device           = Event::Device::kMouse;
    event.position2D       = pos;
    event.flags.Set(Event::Flag::kPosition2D);

    if (phase == DIPhase::kStart) {
        event.flags.Set(Event::Flag::kButtonPress);
        event.button = Event::Button::kMouse1;
    }
    else if (phase == DIPhase::kEnd) {
        event.flags.Set(Event::Flag::kButtonRelease);
        event.button = Event::Button::kMouse1;
    }

    events_.push_back(event);
}

void Emitter_::AddKey(const std::string &key, const KModifiers &modifiers) {
    Event event;
    event.device    = Event::Device::kKeyboard;
    event.key_name  = key;
    event.modifiers = modifiers;

    // Press.
    event.flags.Set(Event::Flag::kKeyPress);
    events_.push_back(event);

    // Release.
    event.flags.Reset(Event::Flag::kKeyPress);
    event.flags.Set(Event::Flag::kKeyRelease);
    events_.push_back(event);
}

void Emitter_::AddControllerPos(Hand hand, const Point3f &pos,
                                const Rotationf &rot) {
    Event event;
    event.device = hand == Hand::kLeft ?
        Event::Device::kLeftController : Event::Device::kRightController;

    event.flags.Set(Event::Flag::kPosition3D);
    event.position3D = pos + (hand == Hand::kLeft ?
                              kLeftControllerPos  + kLeftControllerOffset :
                              kRightControllerPos + kRightControllerOffset);

    event.flags.Set(Event::Flag::kOrientation);
    event.orientation = rot;

    events_.push_back(event);
}

void Emitter_::AddHeadsetButton(bool is_press) {
    Event event;
    event.is_modified_mode = is_mod_;
    event.device           = Event::Device::kHeadset;
    event.flags.Set(
        is_press ? Event::Flag::kButtonPress : Event::Flag::kButtonRelease);
    event.button = Event::Button::kHeadset;

    events_.push_back(event);
}

void Emitter_::EmitEvents(std::vector<Event> &events) {
    // Emit the first event, if any.
    if (! events_.empty()) {
        const Event &event = events_.front();

        // If this is the end of a click (release just after press), delay
        // until after the click timeout. Set a flag so the next time will not
        // delay.
        if (prev_was_button_press_ &&
            event.flags.Has(Event::Flag::kButtonRelease)) {
            if (! waited_for_click_) {
                Util::DelayThread(TK::kMouseClickTimeout);
                waited_for_click_ = true;
                return;
            }
            waited_for_click_ = false;
        }

        prev_was_button_press_ = event.flags.Has(Event::Flag::kButtonPress);

        events.push_back(event);
        events_.pop_front();
    }
}

DECL_SHARED_PTR(Emitter_);

// ----------------------------------------------------------------------------
// MockFilePathList_ class.
// ----------------------------------------------------------------------------

/// Derived FilePathList class that simulates a file system.
class MockFilePathList_ : public FilePathList {
    /// Redefines this to simulate files.
    virtual void GetContents(std::vector<std::string> &subdirs,
                             std::vector<std::string> &files,
                             const std::string &extension,
                             bool include_hidden) const override;
    virtual bool IsValidDirectory(const FilePath &path) const {
        const std::string fn = path.GetFileName();
        return ion::base::StartsWith(fn, "Dir") || fn == "stl" || fn == "maker";
    }
    virtual bool IsExistingFile(const FilePath &path) const {
        return true;
    }
};

void MockFilePathList_::GetContents(std::vector<std::string> &subdirs,
                                    std::vector<std::string> &files,
                                    const std::string &extension,
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

// ----------------------------------------------------------------------------
/// SnapshotApp_ class.
// ----------------------------------------------------------------------------

/// Derived Application class that adds snapshot processing. Reads a SnapScript
/// that specifies what to do.
class SnapshotApp_ : public Application {
  public:
    /// This struct adds some additional options.
    struct Options : public Application::Options {
        SnapScript script;
        bool       nosnap = false;
        bool       remain = false;
    };

    bool Init(const Options &options);

    virtual bool ProcessFrame(size_t render_count, bool force_poll) override;

  private:
    Vector2i        window_size_;   // From Options.
    Emitter_Ptr     emitter_;       // To simulate mouse clicks/drags and keys.
    TestContext     test_context_;  // For accessing managers.
    SnapScript      script_;
    size_t          cur_instruction_ = 0;
    bool            remain_ = false;
    bool            nosnap_ = false;

    bool ProcessInstruction_(const SnapScript::Instr &instr);
    bool LoadSession_(const std::string &file_name);
    bool SetHand_(Hand hand, const std::string &controller_type);
    void SetTouchMode_(bool is_on);
    bool TakeSnapshot_(const Range2f &rect, const std::string &file_name);
    Selection BuildSelection_(const std::vector<std::string> &names);

    template <typename T>
    const T & GetTypedInstr_(const SnapScript::Instr &instr) {
        return static_cast<const T &>(instr);
    }
};

bool SnapshotApp_::Init(const Options &options) {
    if (! Application::Init(options))
        return false;

    nosnap_ = options.nosnap;
    remain_ = options.remain;
    script_ = options.script;

    emitter_.reset(new Emitter_);
    AddEmitter(emitter_);

    window_size_ = GetWindowSize();
    GetTestContext(test_context_);

    // Do NOT write out settings that change because of a script.
    test_context_.settings_manager->SetWriteFlag(false);

    // Turn off controllers until they are specifically added.
    test_context_.scene_context->left_controller->SetEnabled(false);
    test_context_.scene_context->right_controller->SetEnabled(false);

    // Make sure there is no debug text visible.
    Debug::DisplayDebugText("");

    // Ignore mouse events from GLFWViewer so they do not interfere with the
    // click and drag events.
    EnableMouseMotionEvents(false);

    // No need to ask before quitting this app.
    SetAskBeforeQuitting(false);

    // Use the MockFilePathList_ for the FilePanel and ImportToolPanel.
    const auto set_mock = [&](const std::string &panel_name){
        auto panel =
            test_context_.panel_manager->GetTypedPanel<FilePanel>(panel_name);
        panel->SetFilePathList(new MockFilePathList_);
    };
    set_mock("FilePanel");
    set_mock("ImportToolPanel");

    // Set the render offsets for the controllers.
    SetControllerRenderOffsets(-kLeftControllerOffset, -kRightControllerOffset);

    // Use default settings file so that state is deterministic.
    const FilePath path("PublicDoc/snaps/settings/Settings" +
                        TK::kDataFileSuffix);
    if (! test_context_.settings_manager->ReplaceSettings(path)) {
        std::cerr << "*** Unable to load default settings from "
                  << path.ToString() << "\n";
        return false;
    }
    // Tell the SessionManager to update its previous path.
    test_context_.session_manager->ChangePreviousPath(
        test_context_.settings_manager->GetSettings().GetLastSessionPath());

    return true;
}

bool SnapshotApp_::ProcessFrame(size_t render_count, bool force_poll) {
    const size_t instr_count = script_.GetInstructions().size();
    const bool are_more_instructions = cur_instruction_ < instr_count;
    bool keep_going;

    // Let the base class check for exit. Force it to poll for events if there
    // are instructions left to process or if the window is supposed to go
    // away; don't want to wait for an event to come along.
    if (! Application::ProcessFrame(render_count,
                                    are_more_instructions || ! remain_)) {
        keep_going = false;
    }
    // If there are events pending, process them before doing any more
    // instructions.
    else if (emitter_->HasPendingEvents()) {
        keep_going = true;
    }
    // Process the next instruction, if any.
    else if (are_more_instructions) {
        const auto &instr = *script_.GetInstructions()[cur_instruction_];
        keep_going = ProcessInstruction_(instr);
        if (instr.type == SnapScript::Instr::Type::kStop)
            cur_instruction_ = instr_count;
        else
            ++cur_instruction_;
    }
    else {
        // No instructions left: stop ignoring mouse events from GLFWViewer and
        // exit unless the remain flag is set.
        EnableMouseMotionEvents(true);
        keep_going = remain_;
    }
    return keep_going;
}

bool SnapshotApp_::ProcessInstruction_(const SnapScript::Instr &instr) {
    using SIType = SnapScript::Instr::Type;

    const size_t instr_count = script_.GetInstructions().size();
    std::cout << "  Processing " << Util::EnumToWords(instr.type)
              << " (instruction " << (cur_instruction_ + 1)
              << " of " << instr_count << ") on line "
              << instr.line_number << "\n";

    switch (instr.type) {
      case SIType::kAction: {
          const auto &ainst = GetTypedInstr_<SnapScript::ActionInstr>(instr);
          ASSERTM(test_context_.action_manager->CanApplyAction(ainst.action),
                  Util::EnumName(ainst.action));
          test_context_.action_manager->ApplyAction(ainst.action);
          break;
      }
      case SIType::kClick: {
          const auto &cinst = GetTypedInstr_<SnapScript::ClickInstr>(instr);
          emitter_->AddClick(cinst.pos);
          break;
      }
      case SIType::kDrag: {
          const auto &dinst = GetTypedInstr_<SnapScript::DragInstr>(instr);
          emitter_->AddDragPoint(dinst.phase, dinst.pos);
          break;
      }
      case SIType::kHand: {
          const auto &hinst = GetTypedInstr_<SnapScript::HandInstr>(instr);
          if (! SetHand_(hinst.hand, hinst.controller))
              return false;
          break;
      }
      case SIType::kHandPos: {
          const auto &hinst = GetTypedInstr_<SnapScript::HandPosInstr>(instr);
          emitter_->AddControllerPos(hinst.hand, hinst.pos, hinst.rot);
          break;
      }
      case SIType::kHeadset: {
          const auto &hinst = GetTypedInstr_<SnapScript::HeadsetInstr>(instr);
          emitter_->AddHeadsetButton(hinst.is_on);
          break;
      }
      case SIType::kHover: {
          const auto &hinst = GetTypedInstr_<SnapScript::HoverInstr>(instr);
          emitter_->AddHoverPoint(hinst.pos);
          break;
      }
      case SIType::kKey: {
          const auto &kinst = GetTypedInstr_<SnapScript::KeyInstr>(instr);
          Emitter_::KModifiers modifiers;
          if (kinst.is_ctrl_on)
              modifiers.Set(Event::ModifierKey::kControl);
          if (kinst.is_alt_on)
              modifiers.Set(Event::ModifierKey::kAlt);
          emitter_->AddKey(kinst.key, modifiers);
          break;
      }
      case SIType::kLoad: {
          const auto &linst = GetTypedInstr_<SnapScript::LoadInstr>(instr);
          if (! LoadSession_(linst.file_name))
              return false;
          break;
      }
      case SIType::kMod: {
          const auto &minst = GetTypedInstr_<SnapScript::ModInstr>(instr);
          emitter_->SetModifiedMode(minst.is_on);
          break;
      }
      case SIType::kSelect: {
          const auto &sinst = GetTypedInstr_<SnapScript::SelectInstr>(instr);
          test_context_.selection_manager->ChangeSelection(
              BuildSelection_(sinst.names));
          break;
      }
      case SIType::kSettings: {
          const auto &sinst = GetTypedInstr_<SnapScript::SettingsInstr>(instr);
          const FilePath path("PublicDoc/snaps/settings/" + sinst.file_name +
                              TK::kDataFileSuffix);
          if (! test_context_.settings_manager->ReplaceSettings(path))
              return false;
          break;
      }
      case SIType::kSnap: {
          if (! nosnap_) {
              const auto &sinst = GetTypedInstr_<SnapScript::SnapInstr>(instr);
              if (! TakeSnapshot_(sinst.rect, sinst.file_name))
                  return false;
          }
          break;
      }
      case SIType::kStage: {
          const auto &sinst = GetTypedInstr_<SnapScript::StageInstr>(instr);
          auto &stage = *test_context_.scene_context->stage;
          stage.SetScaleAndRotation(sinst.scale, sinst.angle);
          break;
      }
      case SIType::kStop: {
          // Handled elsewhere.
          break;
      }
      case SIType::kTouch: {
          const auto &tinst = GetTypedInstr_<SnapScript::TouchInstr>(instr);
          SetTouchMode_(tinst.is_on);
          break;
      }
      case SIType::kView: {
          const auto &vinst = GetTypedInstr_<SnapScript::ViewInstr>(instr);
          test_context_.scene_context->window_camera->SetOrientation(
              Rotationf::RotateInto(-Vector3f::AxisZ(), vinst.dir));
          break;
      }
      default:
        ASSERTM(false, "Unknown instruction type: " +
                Util::ToString(Util::EnumInt(instr.type)));
        return false;
    }
    return true;
}

bool SnapshotApp_::LoadSession_(const std::string &file_name) {
    // Empty file name means start a new session.
    if (file_name.empty()) {
        test_context_.session_manager->NewSession();
        std::cout << "    Started new session\n";
    }
    else {
        const FilePath path("PublicDoc/snaps/sessions/" + file_name +
                            TK::kSessionFileSuffix);
        std::string error;
        if (! test_context_.session_manager->LoadSession(path, error)) {
            std::cerr << "*** Error loading session from '"
                      << path.ToString() << "':" << error << "\n";
            return false;
        }
        std::cout << "    Loaded session from '" << path.ToString() << "'\n";
    }
    return true;
}

bool SnapshotApp_::SetHand_(Hand hand, const std::string &controller_type) {
    const auto &sc = *test_context_.scene_context;
    auto &controller = hand == Hand::kLeft ?
        *sc.left_controller : *sc.right_controller;

    if (controller_type == "None") {
        controller.SetEnabled(false);
        return true;
    }

    const std::string file_start = "models/controllers/" + controller_type +
        "_" + Util::EnumToWord(hand);

    const std::string mesh_file = file_start + ".tri";
    const std::string tex_file  = file_start + ".jpg";

    std::string mesh_data;
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

void SnapshotApp_::SetTouchMode_(bool is_on) {
    auto &sc = *test_context_.scene_context;

    ForceTouchMode(is_on);

    // Tell the Boards.
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

bool SnapshotApp_::TakeSnapshot_(const Range2f &rect,
                                 const std::string &file_name) {
    const auto &minp = rect.GetMinPoint();
    const auto  size = rect.GetSize();

    const int x = static_cast<int>(minp[0] * window_size_[0]);
    const int y = static_cast<int>(minp[1] * window_size_[1]);
    const int w = static_cast<int>(size[0] * window_size_[0]);
    const int h = static_cast<int>(size[1] * window_size_[1]);

    const auto recti = Range2i::BuildWithSize(Point2i(x, y), Vector2i(w, h));
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

Selection SnapshotApp_::BuildSelection_(const std::vector<std::string> &names) {
    const auto &root_model = test_context_.scene_context->root_model;
    Selection sel;
    for (const auto &name: names) {
        SelPath path(SG::FindNodePathUnderNode(root_model, name, false));
        sel.Add(path);
    }
    return sel;
}

// ----------------------------------------------------------------------------
// Mainline.
// ----------------------------------------------------------------------------

static const char kUsageString[] =
R"(snapimage: Reads a script with instructions on how to create snapshot images
 for public documentation. See SnapScript.h for script details.
    Usage:
      snapimage [--fullscreen] [--klog=<klog_string>]
                [--remain] [--nosnap] SCRIPT

    Options:
      --fullscreen    Start with a full-screen window.
      --klog=<string> String to pass to KLogger::SetKeyString().
      --nosnap        Ignore snap commands (useful for testing).
      --remain        Keep the window alive after script processing.

    Script files are relative to PublicDoc/snaps/scripts.
    Image files are placed in PublicDoc/docs/images.
)";

int main(int argc, const char *argv[]) {
    Args args(argc, argv, kUsageString);

    SnapshotApp_::Options options;

    const FilePath path("PublicDoc/snaps/scripts/" + args.GetString("SCRIPT"));
    if (! options.script.ReadScript(path))
        return -1;

    std::cout << "======= Processing Script file " << path.ToString() << "\n";

    KLogger::SetKeyString(args.GetString("--klog"));
    options.do_ion_remote      = true;
    options.enable_vr          = true;   // So controllers work properly.
    options.fullscreen         = args.GetBool("--fullscreen");
    options.nosnap             = args.GetBool("--nosnap");
    options.remain             = args.GetBool("--remain");
    options.show_session_panel = false;

    // Note that this must have the same aspect ratio as fullscreen.
    options.window_size.Set(1024, 552);

    SnapshotApp_ app;
    if (! app.Init(options))
        return -1;

    try {
        app.MainLoop();
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

    return 0;
}
