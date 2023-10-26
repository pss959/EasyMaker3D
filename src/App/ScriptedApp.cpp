#include "App/ScriptedApp.h"

#include <deque>
#include <string>
#include <vector>

#include <ion/gfx/image.h>
#include <ion/image/conversionutils.h>
#include <ion/math/rangeutils.h>

#include "Base/IEmitter.h"

#include "App/ActionProcessor.h"
#include "Debug/Shortcuts.h"
#include "Items/Controller.h"
#include "Items/Settings.h"
#include "Managers/BoardManager.h"
#include "Managers/CommandManager.h"
#include "Managers/PanelManager.h"
#include "Managers/SceneContext.h"
#include "Managers/SelectionManager.h"
#include "Managers/SessionManager.h"
#include "Managers/SettingsManager.h"
#include "Math/Linear.h"
#include "Models/RootModel.h"
#include "Panels/Board.h"
#include "Panels/FilePanel.h"
#include "Panels/KeyboardPanel.h"
#include "SG/Search.h"
#include "SG/VRCamera.h"
#include "SG/WindowCamera.h"
#include "Selection/Selection.h"
#include "Util/Assert.h"
#include "Util/Delay.h"
#include "Util/FilePath.h"
#include "Util/Read.h"
#include "Util/Tuning.h"
#include "Util/Write.h"
#include "Viewers/Renderer.h"
#include "Widgets/StageWidget.h"

// ----------------------------------------------------------------------------
// ScriptedApp::Emitter_ class.
// ----------------------------------------------------------------------------

/// ScriptedApp::Emitter_ is a derived IEmitter class used to create events to
/// simulate mouse clicks, mouse drags and key presses.
class ScriptedApp::Emitter_ : public IEmitter {
  public:
    using DIPhase    = SnapScript::DragPInstr::Phase;
    using KModifiers = Util::Flags<Event::ModifierKey>;

    /// Sets modified mode for subsequent clicks and drags. It is off by
    /// default.
    void SetModifiedMode(bool is_on) { is_mod_ = is_on; }

    /// Adds a click to emit.
    void AddClick(const Point2f &pos);

    /// Adds an event to simulate a mouse hover at a given position.
    void AddHoverPoint(const Point2f &pos);

    /// Adds a point for a single drag phase to emit.
    void AddDragPoint(DIPhase phase, const Point2f &pos);

    /// Adds points for a drag from \p pos0 to \p pos1 with \p count
    /// intermediate points to emit.
    void AddDragPoints(const Point2f &pos0, const Point2f &pos1, size_t count);

    /// Adds a key press/release to simulate.
    void AddKey(const Str &key, const KModifiers &modifiers);

    /// Adds a controller position.
    void AddControllerPos(Hand hand, const Point3f &pos, const Rotationf &rot);

    /// Adds a VR headset button press or release.
    void AddHeadsetButton(bool is_press);

    /// Returns true if there are events left to process.
    bool HasPendingEvents() const { return ! events_.empty(); }

    virtual void EmitEvents(std::vector<Event> &events) override;
    virtual void FlushPendingEvents() {}

    /// Default rest position for the left controller.
    static constexpr Point3f kLeftControllerPos{-.18f, 14.06, 59.5f};

    /// Default rest position for the right controller.
    static constexpr Point3f kRightControllerPos{.18f, 14.06, 59.5f};

    /// Offset to add to left controller position for event position.
    static constexpr Vector3f kLeftControllerOffset{0, -.12f, 0};

    /// Offset to add to right controller position for event position.
    static constexpr Vector3f kRightControllerOffset{0, .12f, 0};

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

// ----------------------------------------------------------------------------
// ScriptedApp::Emitter_ functions.
// ----------------------------------------------------------------------------

void ScriptedApp::Emitter_::AddClick(const Point2f &pos) {
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

void ScriptedApp::Emitter_::AddHoverPoint(const Point2f &pos) {
    Event event;
    event.is_modified_mode = is_mod_;
    event.device           = Event::Device::kMouse;
    event.position2D       = pos;
    event.flags.Set(Event::Flag::kPosition2D);
    events_.push_back(event);
}

void ScriptedApp::Emitter_::AddDragPoint(DIPhase phase, const Point2f &pos) {
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

void ScriptedApp::Emitter_::AddDragPoints(const Point2f &pos0,
                                          const Point2f &pos1, size_t count) {
    AddDragPoint(DIPhase::kStart, pos0);

    // Add intermediate points, including pos1.
    const float delta = 1.f / (count + 1);
    for (size_t i = 0; i <= count; ++i)
        AddDragPoint(DIPhase::kContinue, Lerp((i + 1) * delta, pos0, pos1));

    AddDragPoint(DIPhase::kEnd, pos1);

#if XXXX
    Event base_event;
    base_event.is_modified_mode = is_mod_;
    base_event.device = Event::Device::kMouse;
    base_event.flags.Set(Event::Flag::kPosition2D);

    // Drag start.
    Event start = base_event;
    start.flags.Set(Event::Flag::kButtonPress);
    start.button     = Event::Button::kMouse1;
    start.position2D = pos0;
    events_.push_back(start);

    // Intermediate points (ending with pos1).
    std::vector<Point2f> pts;
    const float delta = 1.f / (count + 1);
    for (size_t i = 0; i < count; ++i)
        pts.push_back(Lerp((i + 1) * delta, pos0, pos1));
    pts.push_back(pos1);
    Event inter = base_event;
    for (const auto &pt: pts) {
        inter.position2D = pt;
        events_.push_back(inter);
    }

    // Drag end.
    Event end = base_event;
    end.flags.Set(Event::Flag::kButtonRelease);
    end.button     = Event::Button::kMouse1;
    end.position2D = pos1;
    events_.push_back(end);
#endif
}

void ScriptedApp::Emitter_::AddKey(const Str &key,
                                   const KModifiers &modifiers) {
    Event event;
    event.device    = Event::Device::kKeyboard;
    event.key_name  = key;
    event.modifiers = modifiers;
    event.key_text  = Event::BuildKeyText(modifiers, key);

    // Press.
    event.flags.Set(Event::Flag::kKeyPress);
    events_.push_back(event);

    // Release.
    event.flags.Reset(Event::Flag::kKeyPress);
    event.flags.Set(Event::Flag::kKeyRelease);
    events_.push_back(event);
}

void ScriptedApp::Emitter_::AddControllerPos(Hand hand, const Point3f &pos,
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

void ScriptedApp::Emitter_::AddHeadsetButton(bool is_press) {
    Event event;
    event.is_modified_mode = is_mod_;
    event.device           = Event::Device::kHeadset;
    event.flags.Set(
        is_press ? Event::Flag::kButtonPress : Event::Flag::kButtonRelease);
    event.button = Event::Button::kHeadset;

    events_.push_back(event);
}

void ScriptedApp::Emitter_::EmitEvents(std::vector<Event> &events) {
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

// ----------------------------------------------------------------------------
/// ScriptedApp functions.
// ----------------------------------------------------------------------------

bool ScriptedApp::Init(const Options &options) {
    if (! Application::Init(options))
        return false;

    options_ = options;

    emitter_.reset(new Emitter_);
    AddEmitter(emitter_);

    window_size_ = GetWindowSize();

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

    // No need to ask before quitting this app.
    SetAskBeforeQuitting(false);

    // Set the render offsets for the controllers.
    SetControllerRenderOffsets(-Emitter_::kLeftControllerOffset,
                               -Emitter_::kRightControllerOffset);

    // Use default settings file so that state is deterministic.
    const FilePath path("PublicDoc/snaps/settings/Settings" +
                        TK::kDataFileExtension);
    if (! context.settings_manager->SetPath(path, false)) {
        std::cerr << "*** Unable to load default settings from "
                  << path.ToString() << ": "
                  << context.settings_manager->GetLoadError() << "\n";
        return false;
    }
    // Tell the SessionManager to update its previous path.
    context.session_manager->ChangePreviousPath(
        context.settings_manager->GetSettings().GetLastSessionPath());

    return true;
}

bool ScriptedApp::ProcessFrame(size_t render_count, bool force_poll) {
    const size_t instr_count = options_.script.GetInstructions().size();
    const bool are_more_instructions = cur_instruction_ < instr_count;
    bool keep_going;

    // Let the base class check for exit. Force it to poll for events if there
    // are instructions left to process or if the window is supposed to go
    // away; don't want to wait for an event to come along.
    if (! Application::ProcessFrame(
            render_count, are_more_instructions || ! options_.remain)) {
        keep_going = false;
    }
    // If there are events pending, process them before doing any more
    // instructions.
    else if (emitter_->HasPendingEvents()) {
        keep_going = true;
    }
    // Process the next instruction, if any.
    else if (are_more_instructions) {
        const auto &instr = *options_.script.GetInstructions()[cur_instruction_];
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
        keep_going = options_.remain;
    }
    return keep_going;
}

bool ScriptedApp::ProcessInstruction_(const SnapScript::Instr &instr) {
    const size_t instr_count = options_.script.GetInstructions().size();
    if (options_.report)
        std::cout << "  Processing " << Util::EnumToWords(instr.type)
                  << " (instruction " << (cur_instruction_ + 1)
                  << " of " << instr_count << ") on line "
                  << instr.line_number << "\n";

    // Skip snap instructions if disabled.
    if (options_.nosnap && (instr.type == SnapScript::Instr::Type::kSnap ||
                            instr.type == SnapScript::Instr::Type::kSnapObj))
        return true;

    switch (instr.type) {
        using enum SnapScript::Instr::Type;

      case kAction: {
          const auto &ainst = GetTypedInstr_<SnapScript::ActionInstr>(instr);
          ASSERTM(GetContext().action_processor->CanApplyAction(ainst.action),
                  Util::EnumName(ainst.action));
          GetContext().action_processor->ApplyAction(ainst.action);
          break;
      }
      case kClick: {
          const auto &cinst = GetTypedInstr_<SnapScript::ClickInstr>(instr);
          emitter_->AddClick(cinst.pos);
          break;
      }
      case kDrag: {
          const auto &dinst = GetTypedInstr_<SnapScript::DragInstr>(instr);
          emitter_->AddHoverPoint(dinst.pos0);
          emitter_->AddDragPoints(dinst.pos0, dinst.pos1, dinst.count);
          break;
      }
      case kDragP: {
          const auto &dinst = GetTypedInstr_<SnapScript::DragPInstr>(instr);
          emitter_->AddDragPoint(dinst.phase, dinst.pos);
          break;
      }
      case kFocus: {
          const auto &finst = GetTypedInstr_<SnapScript::FocusInstr>(instr);
          FocusPane_(finst.pane_name);
          break;
      }
      case kHand: {
          const auto &hinst = GetTypedInstr_<SnapScript::HandInstr>(instr);
          if (! SetHand_(hinst.hand, hinst.controller))
              return false;
          break;
      }
      case kHandPos: {
          const auto &hinst = GetTypedInstr_<SnapScript::HandPosInstr>(instr);
          emitter_->AddControllerPos(hinst.hand, hinst.pos, hinst.rot);
          break;
      }
      case kHeadset: {
          const auto &hinst = GetTypedInstr_<SnapScript::HeadsetInstr>(instr);
          emitter_->AddHeadsetButton(hinst.is_on);
          break;
      }
      case kHover: {
          const auto &hinst = GetTypedInstr_<SnapScript::HoverInstr>(instr);
          emitter_->AddHoverPoint(hinst.pos);
          break;
      }
      case kKey: {
          const auto &kinst = GetTypedInstr_<SnapScript::KeyInstr>(instr);
          emitter_->AddKey(kinst.key_name, kinst.modifiers);
          break;
      }
      case kLoad: {
          const auto &linst = GetTypedInstr_<SnapScript::LoadInstr>(instr);
          if (! LoadSession_(linst.file_name))
              return false;
          break;
      }
      case kMod: {
          const auto &minst = GetTypedInstr_<SnapScript::ModInstr>(instr);
          emitter_->SetModifiedMode(minst.is_on);
          break;
      }
      case kSelect: {
          const auto &sinst = GetTypedInstr_<SnapScript::SelectInstr>(instr);
          Selection sel;
          BuildSelection_(sinst.names, sel);
          GetContext().selection_manager->ChangeSelection(sel);
          break;
      }
      case kSettings: {
          const auto &sinst = GetTypedInstr_<SnapScript::SettingsInstr>(instr);
          const FilePath path("PublicDoc/snaps/settings/" + sinst.file_name +
                              TK::kDataFileExtension);
          auto &settings_manager = *GetContext().settings_manager;
          if (! settings_manager.SetPath(path, false)) {
              std::cerr << "*** Unable to load settings from "
                        << path.ToString() << ": "
                        << settings_manager.GetLoadError() << "\n";
              return false;
          }
          break;
      }
      case kSnap: {
          const auto &sinst = GetTypedInstr_<SnapScript::SnapInstr>(instr);
          if (! TakeSnapshot_(sinst.rect, sinst.file_name))
              return false;
          break;
      }
      case kSnapObj: {
          const auto &sinst = GetTypedInstr_<SnapScript::SnapObjInstr>(instr);
          Range2f rect;
          if (! GetObjRect_(sinst.object_name, sinst.margin, rect) ||
              ! TakeSnapshot_(rect, sinst.file_name))
              return false;
          break;
      }
      case kStage: {
          const auto &sinst = GetTypedInstr_<SnapScript::StageInstr>(instr);
          auto &stage = *GetContext().scene_context->stage;
          stage.SetScaleAndRotation(sinst.scale, sinst.angle);
          break;
      }
      case kStop: {
          // Handled elsewhere.
          break;
      }
      case kTouch: {
          const auto &tinst = GetTypedInstr_<SnapScript::TouchInstr>(instr);
          SetTouchMode_(tinst.is_on);
          break;
      }
      case kView: {
          const auto &vinst = GetTypedInstr_<SnapScript::ViewInstr>(instr);
          GetContext().scene_context->window_camera->SetOrientation(
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

bool ScriptedApp::LoadSession_(const Str &file_name) {
    // Empty file name means start a new session.
    if (file_name.empty()) {
        GetContext().session_manager->NewSession();
        std::cout << "    Started new session\n";
    }
    else {
        const FilePath path("PublicDoc/snaps/sessions/" + file_name +
                            TK::kSessionFileExtension);
        Str error;
        if (! GetContext().session_manager->LoadSession(path, error)) {
            std::cerr << "*** Error loading session from '"
                      << path.ToString() << "':" << error << "\n";
            return false;
        }
        std::cout << "    Loaded session from '" << path.ToString() << "'\n";
    }
    return true;
}

bool ScriptedApp::FocusPane_(const Str &pane_name) {
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
    auto pane = panel->GetPane()->FindSubPane(pane_name);
    if (! pane || ! pane->GetInteractor()) {
        std::cerr << "*** Could not find interactive pane '" << pane_name
                  << "' in current board\n";
        return false;
    }
    panel->SetFocusedPane(pane);
    return true;
}

bool ScriptedApp::SetHand_(Hand hand, const Str &controller_type) {
    const auto &sc = *GetContext().scene_context;
    auto &controller = hand == Hand::kLeft ?
        *sc.left_controller : *sc.right_controller;

    if (controller_type == "None") {
        controller.SetEnabled(false);
        return true;
    }

    const Str file_start = "models/controllers/" + controller_type +
        "_" + Util::EnumToWord(hand);

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

void ScriptedApp::SetTouchMode_(bool is_on) {
    auto &sc = *GetContext().scene_context;

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

bool ScriptedApp::TakeSnapshot_(const Range2f &rect, const Str &file_name) {
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

bool ScriptedApp::GetObjRect_(const Str &object_name, float margin,
                              Range2f &rect) {
    // Search in the scene for the object.
    const auto &sc = *GetContext().scene_context;
    const auto path = SG::FindNodePathInScene(*sc.scene, object_name, true);
    if (path.empty()) {
        std::cerr << "*** No object named '" << object_name
                  << "' found for snapobj\n";
        return false;
    }

    // Compute the world-coordinate bounds of the object.
    Matrix4f ctm = Matrix4f::Identity();
    for (auto &node: path)
        ctm = ctm * node->GetModelMatrix();
    const auto bounds = TransformBounds(path.back()->GetBounds(), ctm);

    // Find the projection of each bounds corner point on the image plane to
    // get the extents of the rectangle.
    Point3f corners[8];
    bounds.GetCorners(corners);
    rect.MakeEmpty();
    const auto &frustum = *GetContext().scene_context->frustum;
    for (const auto &corner: corners)
        rect.ExtendByPoint(frustum.ProjectToImageRect(corner));

    // Add the margin.
    const Vector2f margin_vec(margin, margin);
    rect.Set(rect.GetMinPoint() - margin_vec, rect.GetMaxPoint() + margin_vec);

    // Clamp to (0,1) in both dimensions.
    rect = RangeIntersection(rect, Range2f(Point2f(0, 0), Point2f(1, 1)));
    return true;
}

void ScriptedApp::BuildSelection_(const StrVec &names, Selection &selection) {
    selection.Clear();
    const auto &root_model = GetContext().scene_context->root_model;
    for (const auto &name: names) {
        SelPath path(SG::FindNodePathUnderNode(root_model, name, false));
        selection.Add(path);
    }
}
