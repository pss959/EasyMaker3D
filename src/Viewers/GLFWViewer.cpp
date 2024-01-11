#include "Viewers/GLFWViewer.h"

#include <algorithm>

#include <ion/gfx/framebufferobject.h>

#include "Base/Event.h"
#include "Base/FBTarget.h"
#include "Base/GLFWWindowSystem.h"
#include "Math/Frustum.h"
#include "Math/Linear.h"
#include "SG/WindowCamera.h"
#include "Util/Assert.h"
#include "Util/General.h"
#include "Util/KLog.h"
#include "Util/Tuning.h"
#include "Viewers/IRenderer.h"

GLFWViewer::GLFWViewer(const ErrorFunc &error_func) :
    error_func_(error_func), ws_(new GLFWWindowSystem), frustum_(new Frustum) {
    ASSERT(error_func);
}

GLFWViewer::~GLFWViewer() {
    ws_->Terminate();
}

bool GLFWViewer::Init(const Vector2ui &size, bool maximize, bool offscreen) {
    const Str title = TK::kApplicationName + " " + TK::kVersionString;
    if (! ws_->Init(error_func_) ||
        ! ws_->CreateMainWindow(size, title, offscreen))
        return false;

    ws_->SetWindowPosition(Point2ui(600, 100));

    if (maximize)
        ws_->Maximize();

    if (offscreen) {
        fb_target_.reset(new FBTarget);
        fb_target_->Init("GLFW", ws_->GetFramebufferSize(),
                         TK::kNonVRSampleCount);
    }

    return true;
}

Vector2ui GLFWViewer::GetWindowSize() const {
    // See the comment in UpdateFrustum_().
    return ws_->GetFramebufferSize();
}

bool GLFWViewer::IsShiftKeyPressed() const {
    return ws_->IsShiftKeyPressed();
}

void GLFWViewer::Render(const SG::Scene &scene, IRenderer &renderer) {
    UpdateFrustum_();

    ws_->PreRender();
    renderer.SetFBTarget(fb_target_);
    renderer.RenderScene(scene, *frustum_);
    ws_->PostRender();
}

void GLFWViewer::EmitEvents(std::vector<Event> &events) {
    ws_->RetrieveEvents(event_options_, events);

    // Add an exit event if the window was closed by the user.
    if (ws_->WasWindowClosed()) {
        Event event;
        event.device = Event::Device::kMouse;
        event.flags.Set(Event::Flag::kExit);
        events.push_back(event);
    }

    // Compress events if possible.
    if (events.size() > 1U)
        CompressEvents_(events);
}

void GLFWViewer::FlushPendingEvents() {
    ws_->FlushPendingEvents();
}

void GLFWViewer::UpdateFrustum_() {
    ASSERT(camera_);
    // Note: On some displays (ahem, Mac) the window and framebuffer sizes are
    // not the same.
    camera_->BuildFrustum(ws_->GetFramebufferSize(), *frustum_);
}

void GLFWViewer::CompressEvents_(std::vector<Event> &events) {
    ASSERT(events.size() > 1U);

    // It would be nice to be able to use an STL algorithm here.
    // std::remove_if() does not look at neighbors. Using std::unique() or
    // std::unique_copy() does not work because we always have to choose the
    // second Event if two are considered "equivalent", and I don't want to
    // have to reverse the vector twice.

    // Temporarily set serial numbers in the events to make this easier.
    for (size_t i = 0; i < events.size(); ++i)
        events[i].serial = i;

    // Erase an Event if it has only mouse position and the next event also has
    // 2D position.
    auto should_erase = [&events](const Event &ev){
        return ev.serial + 1 < events.size() &&
            ev.flags.HasOnly(Event::Flag::kPosition2D) &&
            events[ev.serial + 1].flags.Has(Event::Flag::kPosition2D);
    };

    const auto it = std::remove_if(events.begin(), events.end(), should_erase);
    if (it != events.end()) {
        KLOG('E', "GLFWViewer compressing " << std::distance(it, events.end())
             << " event(s)");
        events.erase(it, events.end());
    }

    // Clear the serial numbers
    for (Event &event: events)
        event.serial = 0;
}
