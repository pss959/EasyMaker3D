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
#include "Items/Controller.h"
#include "Managers/ActionManager.h"
#include "Managers/CommandManager.h"
#include "Managers/SelectionManager.h"
#include "Managers/SessionManager.h"
#include "Managers/SettingsManager.h"
#include "Math/Types.h"
#include "Models/RootModel.h"
#include "SG/Search.h"
#include "SG/WindowCamera.h"
#include "Tests/TestContext.h"
#include "Util/Assert.h"
#include "Util/Delay.h"
#include "Util/FilePath.h"
#include "Util/KLog.h"
#include "Util/Read.h"
#include "Util/Write.h"
#include "Widgets/StageWidget.h"

// ----------------------------------------------------------------------------
// Emitter_ class.
// ----------------------------------------------------------------------------

/// Emitter_ is a derived IEmitter class used to create events to simulate
/// mouse clicks, mouse drags and key presses.
class Emitter_ : public IEmitter {
  public:
    using DIPhase = SnapScript::DragInstr::Phase;
    using KMods   = Util::Flags<Event::ModifierKey>;

    /// Adds a click to emit.
    void AddClick(const Point2f &pos) {
        points_.push_back(Point_(true, DIPhase::kStart, pos));
        points_.push_back(Point_(true, DIPhase::kEnd,   pos));
    }

    /// Adds a drag point to emit.
    void AddDragPoint(DIPhase phase, const Point2f &pos) {
        points_.push_back(Point_(false, phase, pos));
    }

    /// Adds a key press/release to simulate.
    void AddKey(const std::string &key, const KMods &mods) {
        keys_.push_back(Key_(key, mods, true));   // Press.
        keys_.push_back(Key_(key, mods, false));  // Release.
    }

    /// Returns true if there are events left to process.
    bool HasPendingEvents() const {
        return ! points_.empty() || ! keys_.empty();
    }

    virtual void EmitEvents(std::vector<Event> &events) override;
    virtual void FlushPendingEvents() {}

  private:
    /// Struct representing a point to process for a click or drag.
    struct Point_ {
        bool          is_click;
        const DIPhase phase;
        const Point2f pos;
        Point_(bool is_click_in, DIPhase phase_in, const Point2f &pos_in) :
            is_click(is_click_in), phase(phase_in), pos(pos_in) {}
    };

    /// Struct representing a key to process.
    struct Key_ {
        const std::string  key_name;
        const KMods        modifiers;
        bool               is_press;
        Key_(const std::string &key, const KMods &mods, bool press) :
            key_name(key), modifiers(mods), is_press(press) {}
    };

    /// Points left in the current click or drag operation.
    std::deque<Point_> points_;

    /// Keys left to simulate.
    std::deque<Key_>   keys_;

    /// Set to true while waiting for a click to be processed after a timeout.
    bool               is_waiting_for_click_ = false;
};

void Emitter_::EmitEvents(std::vector<Event> &events) {
    // XXXX Figure out how to ignore accidental mouse events from GLFWViewer
    // while dragging. Maybe add "source" field to Event.

    if (! points_.empty()) {
        const auto &pt = points_.front();

        // If this is the end of a click, make sure the click timeout is
        // reached so that it is processed during the next frame and wait until
        // then. Use the is_waiting_for_click_ flag to do this once.
        if (pt.is_click && pt.phase == DIPhase::kEnd) {
            if (! is_waiting_for_click_) {
                is_waiting_for_click_ = true;
                Util::DelayThread(TK::kMouseClickTimeout);
                return;
            }
            else {
                is_waiting_for_click_ = false;
            }
        }

        Event event;
        event.device = Event::Device::kMouse;
        if (pt.phase == DIPhase::kStart) {
            event.flags.Set(Event::Flag::kButtonPress);
            event.button = Event::Button::kMouse1;
        }
        else if (pt.phase == DIPhase::kEnd) {
            event.flags.Set(Event::Flag::kButtonRelease);
            event.button = Event::Button::kMouse1;
        }
        event.flags.Set(Event::Flag::kPosition2D);
        event.position2D = pt.pos;
        events.push_back(event);

        points_.pop_front();
    }

    if (! keys_.empty()) {
        const auto &key = keys_.front();

        Event event;
        event.device = Event::Device::kKeyboard;
        event.flags.Set(key.is_press ?
                        Event::Flag::kKeyPress : Event::Flag::kKeyRelease);
        event.key_name  = key.key_name;
        event.modifiers = key.modifiers;
        events.push_back(event);

        keys_.pop_front();
    }
}

DECL_SHARED_PTR(Emitter_);

// ----------------------------------------------------------------------------
/// SnapshotApp_ class.
// ----------------------------------------------------------------------------

/// Derived Application class that adds snapshot processing. Reads a SnapScript
/// that specifies what to do.
class SnapshotApp_ : public Application {
  public:
    virtual bool Init(const Options &options) override;
    void SetRemain(bool remain) { remain_ = remain; }
    void SetScript(const SnapScript &script) { script_ = script; }

    virtual bool ProcessFrame(size_t render_count, bool force_poll) override;

  private:
    Vector2i        window_size_;   // From Options.
    Emitter_Ptr     emitter_;       // To simulate mouse clicks/drags and keys.
    TestContext     test_context_;  // For accessing managers.
    SnapScript      script_;
    size_t          cur_instruction_ = 0;
    bool            remain_ = false;

    bool ProcessInstruction_(const SnapScript::Instr &instr);
    bool LoadSession_(const std::string &file_name);
    bool AddHand_(Hand hand, const std::string &controller_type,
                  const Point3f &pos, const Vector3f &dir);
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

    emitter_.reset(new Emitter_);
    AddEmitter(emitter_);

    window_size_ = GetWindowSize();
    GetTestContext(test_context_);

    // Make sure there is no debug text visible.
    Debug::DisplayDebugText("");

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
        ++cur_instruction_;
    }
    else {
        // No instructions left - stop unless the remain flag is set.
        keep_going = remain_;
    }
    return keep_going;
}

bool SnapshotApp_::ProcessInstruction_(const SnapScript::Instr &instr) {
    using SIType = SnapScript::Instr::Type;

    const size_t instr_count = script_.GetInstructions().size();
    std::cout << "  Processing " << Util::EnumToWords(instr.type)
              << " (instruction " << (cur_instruction_ + 1)
              << " of " << instr_count << ")\n";

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
          if (! AddHand_(hinst.hand, hinst.controller, hinst.pos, hinst.dir))
              return false;
          break;
      }
      case SIType::kKey: {
          const auto &kinst = GetTypedInstr_<SnapScript::KeyInstr>(instr);
          Emitter_::KMods mods;
          if (kinst.is_ctrl_on)
              mods.Set(Event::ModifierKey::kControl);
          if (kinst.is_alt_on)
              mods.Set(Event::ModifierKey::kAlt);
          emitter_->AddKey(kinst.key, mods);
          break;
      }
      case SIType::kLoad: {
          const auto &linst = GetTypedInstr_<SnapScript::LoadInstr>(instr);
          if (! LoadSession_(linst.file_name))
              return false;
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
          const FilePath path("PublicDoc/snaps/settings/" + sinst.file_name);
          if (! test_context_.settings_manager->ReplaceSettings(path))
              return false;
          break;
      }
      case SIType::kSnap: {
          const auto &sinst = GetTypedInstr_<SnapScript::SnapInstr>(instr);
          if (! TakeSnapshot_(sinst.rect, sinst.file_name))
              return false;
          break;
      }
      case SIType::kStage: {
          const auto &sinst = GetTypedInstr_<SnapScript::StageInstr>(instr);
          auto &stage = *test_context_.scene_context->stage;
          stage.SetScaleAndRotation(sinst.scale, sinst.angle);
          break;
      }
      case SIType::kTouch: {
          const auto &sinst = GetTypedInstr_<SnapScript::TouchInstr>(instr);
          auto &sc = *test_context_.scene_context;
          sc.left_controller->SetTouchMode(sinst.is_on);
          sc.right_controller->SetTouchMode(sinst.is_on);
          ForceTouchMode(sinst.is_on);
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
    const FilePath path("PublicDoc/snaps/sessions/" + file_name);

    std::string error;
    if (! test_context_.session_manager->LoadSession(path, error)) {
        std::cerr << "*** Error loading session from '"
                  << path.ToString() << "':" << error << "\n";
        return false;
    }
    std::cout << "    Loaded session from '" << path.ToString() << "'\n";
    return true;
}

bool SnapshotApp_::AddHand_(Hand hand, const std::string &controller_type,
                            const Point3f &pos, const Vector3f &dir) {
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
    controller.SetTranslation(pos);
    controller.SetRotation(Rotationf::RotateInto(Vector3f(0, 0, -1), dir));

    // Since at least one controller is in use, turn off the RadialMenu parent
    // in the room, since the menus will be attached to the controllers.
    const auto parent = SG::FindNodeInScene(*sc.scene, "RadialMenus");
    parent->SetEnabled(false);

    return true;
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
      snapimage [--fullscreen] [--klog=<klog_string>] [--remain] SCRIPT

    Options:
      --fullscreen    Start with a full-screen window.
      --klog=<string> String to pass to KLogger::SetKeyString().
      --remain        Keep the window alive after script processing

    Script files are relative to PublicDoc/snaps/scripts.
    Image files are placed in PublicDoc/docs/images.
)";

int main(int argc, const char *argv[]) {
    Args args(argc, argv, kUsageString);

    SnapScript script;
    const FilePath path("PublicDoc/snaps/scripts/" + args.GetString("SCRIPT"));
    if (! script.ReadScript(path))
        return -1;

    std::cout << "======= Processing Script file " << path.ToString() << "\n";

    KLogger::SetKeyString(args.GetString("--klog"));
    Application::Options options;
    options.do_ion_remote      = true;
    options.fullscreen         = args.GetBool("--fullscreen");
    options.show_session_panel = false;
    // Note that this must have the same aspect ratio as fullscreen.
    options.window_size.Set(1024, 552);

    SnapshotApp_ app;
    if (! app.Init(options))
        return -1;

    app.SetRemain(args.GetBool("--remain"));
    app.SetScript(script);
    app.MainLoop();

    return 0;
}
