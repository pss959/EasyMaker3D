#include "App/CaptureScriptApp.h"

#include "Debug/Shortcuts.h"
#include "Items/Settings.h"
#include "Managers/SceneContext.h"
#include "Managers/SessionManager.h"
#include "Managers/SettingsManager.h"
#include "Math/Intersection.h"
#include "SG/CoordConv.h"
#include "SG/Node.h"
#include "SG/Scene.h"
#include "SG/Search.h"
#include "SG/WindowCamera.h"
#include "Util/Assert.h"
#include "Util/Tuning.h"

bool CaptureScriptApp::Init(const Options &options) {
    if (! Application::Init(options))
        return false;

    options_ = options;

    window_size_ = GetWindowSize();

    const auto &context = GetContext();

#if ENABLE_DEBUG_FEATURES
    // Make sure there is no debug text visible.
    Debug::DisplayDebugText("");
#endif

    // Ignore mouse events from GLFWViewer so they do not interfere with the
    // click and drag events.
    EnableMouseMotionEvents(false);

    // No need to ask before quitting this app.
    SetAskBeforeQuitting(false);

    // Use default settings file so that state is deterministic.
    const FilePath path("PublicDoc/captures/settings/Settings" +
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

    // Find the FakeCursor and position it just in front of the camera.
    ASSERT(context.scene_context);
    ASSERT(context.scene_context->scene);
    const auto &scene = *context.scene_context->scene;
    cursor_ = SG::FindNodeInScene(scene, "FakeCursor");
    MoveCursorTo_(GetFrustum(), Point2f::Zero());

    return true;
}

bool CaptureScriptApp::ProcessFrame(size_t render_count, bool force_poll) {
    const size_t instr_count = options_.script.GetInstructions().size();
    const bool are_more_instructions = cur_instruction_ < instr_count;
    bool keep_going = true;

    // Let the base class check for exit. Force it to poll for events if there
    // are instructions left to process or if the window is supposed to go
    // away; don't want to wait for an event to come along.
    if (! Application::ProcessFrame(
            render_count, are_more_instructions || ! options_.remain)) {
        keep_going = false;
    }
    // Process the next instruction, if any.
    else if (are_more_instructions) {
        const auto &instr =
            *options_.script.GetInstructions()[cur_instruction_];
        keep_going = ProcessInstruction_(instr);
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

bool CaptureScriptApp::ProcessInstruction_(const CaptureScript::Instr &instr) {
    const size_t instr_count = options_.script.GetInstructions().size();
    if (options_.report)
        std::cout << "  Processing " << instr.name
                  << " (instruction " << (cur_instruction_ + 1)
                  << " of " << instr_count << ") on line "
                  << instr.line_number << "\n";

    if (instr.name == "click") {
        const auto &cinst = GetTypedInstr_<CaptureScript::ClickInstr>(instr);
        // XXXX
        std::cerr << "XXXX INST: " << cinst.name << "\n";
    }
    else if (instr.name == "cursor") {
        const auto &cinst = GetTypedInstr_<CaptureScript::CursorInstr>(instr);
        cursor_->SetEnabled(cinst.is_on);
    }
    else if (instr.name == "drag") {
        const auto &dinst = GetTypedInstr_<CaptureScript::DragInstr>(instr);
        // XXXX
        std::cerr << "XXXX INST: " << dinst.name << "\n";
    }
    else if (instr.name == "mod") {
        const auto &minst = GetTypedInstr_<CaptureScript::ModInstr>(instr);
        // SetModifiedMode(minst.is_on);
        // XXXX
        std::cerr << "XXXX INST: " << minst.name << "\n";
    }
    else if (instr.name == "moveover") {
        const auto &minst = GetTypedInstr_<CaptureScript::MoveOverInstr>(instr);
        MoveCursorOver_(minst.object_name);
    }
    else if (instr.name == "moveto") {
        const auto &minst = GetTypedInstr_<CaptureScript::MoveToInstr>(instr);
        MoveCursorTo_(GetFrustum(), minst.pos);
    }
    else if (instr.name == "wait") {
        const auto &winst = GetTypedInstr_<CaptureScript::WaitInstr>(instr);
        // XXXX
        std::cerr << "XXXX INST: " << winst.name << "\n";
    }
    else {
        ASSERTM(false, "Unknown instruction type: " + instr.name);
        return false;
    }
    return true;
}

void CaptureScriptApp::MoveCursorTo_(const Frustum &frustum,
                                     const Point2f &pos) {
    // Build a ray through the point using the view frustum.
    const auto ray = frustum.BuildRay(pos);

    // Intersect the ray with the plane -1 unit away in Z.
    const Plane plane(frustum.position[2] - 1, Vector3f::AxisZ());
    float distance;
    RayPlaneIntersect(ray, plane, distance);

    cursor_->TranslateTo(ray.GetPoint(distance));
}

void CaptureScriptApp::MoveCursorOver_(const Str &object_name) {
    const auto &root = GetContext().scene_context->scene->GetRootNode();

    // Find the object in the scene. Note that the name may be compound
    // ("A/B/C").
    if (object_name.contains('/')) {
        std::cerr << "XXXX Compound name '" << object_name << "'\n";
    }
    else {
        std::cerr << "XXXX Simple name '" << object_name << "'\n";
        auto path = SG::FindNodePathUnderNode(root, object_name);
        std::cerr << "XXXX   Found path '" << path << "'\n";
        // Find the center of the object in world coordinates.
        const auto center = SG::CoordConv(path).ObjectToRoot(Point3f::Zero());
        std::cerr << "XXXX   Center = " << center << "\n";
        // Project the center onto the Frustum to get the point to move to.
        const Frustum frustum = GetFrustum();
        MoveCursorTo_(frustum, frustum.ProjectToImageRect(center));
    }
}

Frustum CaptureScriptApp::GetFrustum() const {
    // Get the WindowCamera from the Scene and let it build a Frustum.
    auto cam =
        GetContext().scene_context->scene->GetTypedCamera<SG::WindowCamera>();
    ASSERT(cam);
    Frustum frustum;
    cam->BuildFrustum(window_size_, frustum);
    return frustum;
}
