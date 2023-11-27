#include "App/CaptureScriptApp.h"

#include <cmath>
#include <functional>
#include <ranges>

#include "App/CaptureScript.h"
#include "App/ScriptEmitter.h"
#include "Base/Event.h"
#include "Handlers/Handler.h"
#include "Managers/EventManager.h"
#include "Managers/SceneContext.h"
#include "Math/Intersection.h"
#include "SG/CoordConv.h"
#include "SG/Node.h"
#include "SG/Scene.h"
#include "SG/Search.h"
#include "SG/WindowCamera.h"
#include "Util/Assert.h"

// ----------------------------------------------------------------------------
// CaptureScriptApp::CursorHandler_ class.
// ----------------------------------------------------------------------------

/// This derived Handler class is used by the CaptureScriptApp to update the
/// fake cursor when a mouse motion event is seen.
class CaptureScriptApp::CursorHandler_ : public Handler {
  public:
    using EventFunc = std::function<void(const Point2f &)>;

    /// The constructor is passed a function to invoke when a mouse event is
    /// handled.
    explicit CursorHandler_(const EventFunc &func) : event_func_(func) {
        ASSERT(func);
    }

    /// Defines this to invoke the function.
    virtual bool HandleEvent(const Event &event) override {
        if (event.flags.Has(Event::Flag::kPosition2D))
            event_func_(event.position2D);
        return false;  // Let other handlers see the event.
    }

  private:
    EventFunc event_func_;
};

// ----------------------------------------------------------------------------
// CaptureScriptApp functions.
// ----------------------------------------------------------------------------

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
    MoveFakeCursorTo_(Point2f(.5f, .5f));  // In the middle.

    // Set a delay between emitted events.
    GetEmitter().SetDelay(1.f / 30);

    // Add a CursorHandler_ to update the fake cursor when the mouse is moved.
    // Insert it at the beginning so no other handler steals the event.
    handler_.reset(new CursorHandler_(
                       [&](const Point2f &p){ MoveFakeCursorTo_(p); }));
    GetContext().event_manager->InsertHandler(handler_);

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
        MoveCursorOver_(minst.object_name, minst.seconds);
    }
    else if (instr.name == "moveto") {
        const auto &minst = GetTypedInstr_<CaptureScript::MoveToInstr>(instr);
        MoveCursorTo_(minst.pos, minst.seconds);
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

void CaptureScriptApp::InstructionsDone() {
    // Disable the handler so the fake cursor does not move any more.
    handler_->SetEnabled(false);
}

void CaptureScriptApp::MoveCursorOver_(const Str &object_name, float seconds) {
    const auto &root = GetContext().scene_context->scene->GetRootNode();

    // Find the object in the scene. Note that the name may be compound
    // ("A/B/C").
    if (object_name.contains('/')) {
        std::cerr << "XXXX Compound name '" << object_name << "'\n";
        // XXXX Do something...
    }
    else {
        auto path = SG::FindNodePathUnderNode(root, object_name);
        // Project the center of the object in world coordinates onto the
        // Frustum image plane to get the point to move to.
        const auto center = SG::CoordConv(path).ObjectToRoot(Point3f::Zero());
        MoveCursorTo_(GetFrustum().ProjectToImageRect(center), seconds);
    }
}

void CaptureScriptApp::MoveCursorTo_(const Point2f &pos, float seconds) {
    if (seconds > 0 && GetEmitter().GetDelay() > 0) {
        // Determine the number of events to create over the duration.
        const size_t count = std::roundf(seconds / GetEmitter().GetDelay());
        std::cerr << "XXXX seconds = " << seconds
                  << " count = " << count << "\n";
        for (auto i : std::views::iota(0U, count)) {
            GetEmitter().AddHoverPoint(Lerp(cursor_pos_, pos,
                                            static_cast<float>(i + 1) / count));
        }
    }

    // XXXX
}

void CaptureScriptApp::MoveFakeCursorTo_(const Point2f &pos) {
    // Build a ray through the point using the view frustum.
    const Frustum frustum = GetFrustum();
    const auto ray = frustum.BuildRay(pos);

    // Intersect the ray with the plane -1 unit away in Z and translate the
    // fake cursor to the resulting point.
    const Plane plane(frustum.position[2] - 1, Vector3f::AxisZ());
    float distance;
    RayPlaneIntersect(ray, plane, distance);
    cursor_->TranslateTo(ray.GetPoint(distance));

    cursor_pos_ = pos;
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
