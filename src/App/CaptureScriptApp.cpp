#include "App/CaptureScriptApp.h"

#include "App/CaptureScript.h"
#include "App/ScriptEmitter.h"
#include "Managers/SceneContext.h"
#include "Math/Intersection.h"
#include "SG/CoordConv.h"
#include "SG/Node.h"
#include "SG/Scene.h"
#include "SG/Search.h"
#include "SG/WindowCamera.h"
#include "Util/Assert.h"

bool CaptureScriptApp::Init(const OptionsPtr &options,
                            const ScriptBasePtr &script) {
    if (! ScriptedApp::Init(options, script))
        return false;

    const auto &context = GetContext();

    // Find the FakeCursor and position it just in front of the camera.
    ASSERT(context.scene_context);
    ASSERT(context.scene_context->scene);
    const auto &scene = *context.scene_context->scene;
    cursor_ = SG::FindNodeInScene(scene, "FakeCursor");
    MoveCursorTo_(GetFrustum(), Point2f::Zero());

    return true;
}

bool CaptureScriptApp::ProcessInstruction(const ScriptBase::Instr &instr) {
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
    cam->BuildFrustum(GetWindowSize(), frustum);
    return frustum;
}
