#include "App/CaptureScriptApp.h"

#include <cmath>
#include <functional>
#include <ranges>

#include <ion/base/stringutils.h>
#include <ion/gfx/image.h>
#include <ion/image/conversionutils.h>

#include "App/CaptureScript.h"
#include "App/ScriptEmitter.h"
#include "Base/Event.h"
#include "Handlers/Handler.h"
#include "Managers/EventManager.h"
#include "Managers/SceneContext.h"
#include "Math/Intersection.h"
#include "Math/Linear.h"
#include "SG/CoordConv.h"
#include "SG/Node.h"
#include "SG/Scene.h"
#include "SG/Search.h"
#include "App/VideoWriter.h"
#include "SG/WindowCamera.h"
#include "Util/Assert.h"
#include "Viewers/Renderer.h"

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

CaptureScriptApp::CaptureScriptApp() {}

CaptureScriptApp::~CaptureScriptApp() {}

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

    // Set up a VideoWriter if requested.
    if (! GetOptions_().nocapture)
        // XXXX FPS constant somewhere...
        video_writer_.reset(new VideoWriter(GetOptions_().window_size, 30));

    return true;
}

bool CaptureScriptApp::ProcessInstruction(const ScriptBase::Instr &instr) {
    if (instr.name == "click") {
        GetEmitter().AddClick(cursor_pos_);
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

    // Write the resulting video if requested.
    if (video_writer_) {
        const FilePath &script_path = GetScript().GetPath();
        FilePath video_path("PublicDoc/docs/videos/" +
                            script_path.GetFileName());
        video_path.ReplaceExtension(".mp4");
        video_writer_->WriteToFile(video_path);
    }

    // Remove the VideoWriter_ so this does not happen again in case
    // InstructionsDone() is called again (if remain flag is set).
    video_writer_.reset();
}

void CaptureScriptApp::FrameDone() {
    if (video_writer_) {
        const auto image = GetRenderer().ReadImage(
            Range2i::BuildWithSize(Point2i(0, 0), GetWindowSize()));
        // Rows of image need to be inverted (GL vs stblib).
        ion::image::FlipImage(image);
        video_writer_->AddImage(*image);
    }
}

const CaptureScriptApp::Options & CaptureScriptApp::GetOptions_() const {
    const auto &opts = GetOptions();
    ASSERT(dynamic_cast<const Options *>(&opts));
    return static_cast<const Options &>(opts);
}

void CaptureScriptApp::MoveCursorOver_(const Str &object_name, float seconds) {
    SG::NodePtr root = GetContext().scene_context->scene->GetRootNode();
    SG::NodePath path;  // Path from scene root to target object.

    // Find the object in the scene. Note that the name may be compound
    // ("A/B/C").
    if (object_name.contains('/')) {
        const auto parts = ion::base::SplitString(object_name, "/");
        ASSERT(parts.size() > 1U);
        for (const auto &part: parts) {
            auto sub_path = SG::FindNodePathUnderNode(root, part);
            if (path.empty())
                path = sub_path;
            else
                path = SG::NodePath::Stitch(path, sub_path);
            root = path.back();
        }
    }
    else {
        path = SG::FindNodePathUnderNode(root, object_name);
    }

    // Project the center of the object in world coordinates onto the Frustum
    // image plane to get the point to move to.
    const auto center = SG::CoordConv(path).ObjectToRoot(Point3f::Zero());
    MoveCursorTo_(GetFrustum().ProjectToImageRect(center), seconds);
}

void CaptureScriptApp::MoveCursorTo_(const Point2f &pos, float seconds) {
    if (seconds > 0 && GetEmitter().GetDelay() > 0) {
        // Determine the number of events to create over the duration.
        const size_t count = std::roundf(seconds / GetEmitter().GetDelay());
        for (auto i : std::views::iota(0U, count)) {
            const float t = static_cast<float>(i + 1) / count;
            GetEmitter().AddHoverPoint(BezierInterp(t, cursor_pos_, pos));
        }
    }
    else {
        // No duration or no delay - just one point.
        GetEmitter().AddHoverPoint(pos);
    }
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
