#include "App/SnapScriptApp.h"

#include <ion/gfx/image.h>
#include <ion/image/conversionutils.h>

#include "App/ActionProcessor.h"
#include "App/ScriptEmitter.h"
#include "App/SnapScript.h"
#include "Debug/Shortcuts.h"
#include "Items/Controller.h"
#include "Managers/BoardManager.h"
#include "Managers/PanelManager.h"
#include "Managers/SceneContext.h"
#include "Managers/SelectionManager.h"
#include "Managers/SessionManager.h"
#include "Models/RootModel.h"
#include "Panels/Board.h"
#include "Panels/KeyboardPanel.h"
#include "SG/Search.h"
#include "SG/VRCamera.h"
#include "SG/WindowCamera.h"
#include "Selection/Selection.h"
#include "Util/Assert.h"
#include "Util/FilePath.h"
#include "Util/Read.h"
#include "Util/Tuning.h"
#include "Util/Write.h"
#include "Viewers/Renderer.h"
#include "Widgets/StageWidget.h"

bool SnapScriptApp::ProcessInstruction(const Script::Instr &instr) {
    // Skip snap instructions if disabled.
    if (GetOptions_().nosnap &&
        (instr.name == "snap" || instr.name == "snapobj"))
        return true;

    auto &emitter = GetEmitter();

    if (instr.name == "action") {
        const auto &ainst = GetTypedInstr_<Script::ActionInstr>(instr);
        ASSERTM(GetContext().action_processor->CanApplyAction(ainst.action),
                Util::EnumName(ainst.action));
        GetContext().action_processor->ApplyAction(ainst.action);
    }
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
        ASSERTM(false, "Unknown instruction type: " + instr.name);
        return false;
    }
    return true;
}

const SnapScriptApp::Options & SnapScriptApp::GetOptions_() const {
    const auto &opts = GetOptions();
    ASSERT(dynamic_cast<const Options *>(&opts));
    return static_cast<const Options &>(opts);
}

bool SnapScriptApp::LoadSession_(const Str &file_name) {
    // Empty file name means start a new session.
    if (file_name.empty()) {
        GetContext().session_manager->NewSession();
        if (GetOptions_().report)
            std::cout << "    Started new session\n";
    }
    else {
        const FilePath path("PublicDoc/snaps/sessions/" + file_name +
                            TK::kSessionFileExtension);
        Str error;
        if (! GetContext().session_manager->LoadSession(path, error)) {
            std::cerr << "*** Error loading session from '" << path
                      << "':" << error << "\n";
            return false;
        }
        if (GetOptions_().report)
            std::cout << "    Loaded session from '" << path << "'\n";
    }
    return true;
}

bool SnapScriptApp::FocusPane_(const Str &pane_name) {
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

bool SnapScriptApp::SetHand_(Hand hand, const Str &controller_type) {
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

void SnapScriptApp::SetTouchMode_(bool is_on) {
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

bool SnapScriptApp::TakeSnapshot_(const Range2f &rect, const Str &file_name) {
    const auto &minp = rect.GetMinPoint();
    const auto  size = rect.GetSize();

    const auto window_size = GetWindowSize();
    const int x = static_cast<int>(minp[0] * window_size[0]);
    const int y = static_cast<int>(minp[1] * window_size[1]);
    const int w = static_cast<int>(size[0] * window_size[0]);
    const int h = static_cast<int>(size[1] * window_size[1]);

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

void SnapScriptApp::BuildSelection_(const StrVec &names, Selection &selection) {
    selection.Clear();
    const auto &root_model = GetContext().scene_context->root_model;
    for (const auto &name: names) {
        SelPath path(SG::FindNodePathUnderNode(root_model, name, false));
        selection.Add(path);
    }
}
