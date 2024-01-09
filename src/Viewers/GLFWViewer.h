#pragma once

#include <functional>

#include "Base/FBTarget.h"
#include "Base/IEmitter.h"
#include "Base/IWindowSystem.h"
#include "Math/Types.h"
#include "Util/Memory.h"
#include "Viewers/Viewer.h"

DECL_SHARED_PTR(Frustum);
DECL_SHARED_PTR(GLFWViewer);
DECL_SHARED_PTR(IWindowSystem);
namespace SG { DECL_SHARED_PTR(WindowCamera); }

/// GLFWViewer is a derived Viewer and IEmitter that uses the GLFW library
/// to do windowing and input.
///
/// \ingroup Viewers
class GLFWViewer : public Viewer, public IEmitter {
  public:
    using ErrorFunc = std::function<void(const Str &)>;

    /// The constructor is passed a function to call when an error occurs. It
    /// is passed a string containing the error message.
    explicit GLFWViewer(const ErrorFunc &error_func);

    virtual ~GLFWViewer();

    /// Replaces the GLFWWindowSystem used by the GLFWViewer with a different
    /// IWindowSystem instance. This is used for testing without having to use
    /// actual GLFW graphics.
    void SetWindowSystem(const IWindowSystemPtr &ws) { ws_ = ws; }

    /// Initializes the viewer with the given \p size. The \p fullscreen flag
    /// indicates whether the window should be resized to full-screen. The \p
    /// show_window flag indicates whether the window should be visible (can be
    /// false for tests). Returns false if anything fails.
    bool Init(const Vector2i &size, bool fullscreen, bool show_window);

    /// Sets the WindowCamera to update.
    void SetCamera(const SG::WindowCameraPtr &camera) {
        camera_ = camera;
        UpdateFrustum_();
    }

    /// Returns the Frustum computed for the latest render.
    const FrustumPtr & GetFrustum() const { return frustum_; }

    /// Sets a flag indicating whether to wait for events or to poll events
    /// continuously. The default is to wait.
    void SetPollEventsFlag(bool should_poll) {
        event_options_.wait_for_events = ! should_poll;
    }

    /// Returns true if either shift key is currenty pressed.
    bool IsShiftKeyPressed() const;

    /// Returns the current size of the window in pixels, taking the fullscreen
    /// option into account.
    Vector2i GetWindowSize() const;

    virtual void Render(const SG::Scene &scene, IRenderer &renderer);

    virtual void EmitEvents(std::vector<Event> &events) override;
    virtual void FlushPendingEvents() override;

    /// This can be used to temporarily ignore mouse motion events produced by
    /// GLFW. It is used by the snapimage application to avoid spurious events
    /// caused by accidental mouse motion during processing. They are enabled
    /// by default.
    void EnableMouseMotionEvents(bool enable) {
        event_options_.ignore_mouse_motion = ! enable;
    }

  private:
    ErrorFunc        error_func_;
    FBTarget         fb_target_;
    IWindowSystemPtr ws_;

    /// Stores the camera used to set up the Frustum.
    SG::WindowCameraPtr camera_;

    /// Stores the current Frustum for the viewer.
    FrustumPtr frustum_;

    /// Stores options related to emitting events.
    IWindowSystem::EventOptions event_options_;

    /// Updates the Frustum based on the current state.
    void UpdateFrustum_();

    /// Compresses events in the given vector: multiple mouse motion events
    /// with no button presses or releases are compressed into the last one.
    void CompressEvents_(std::vector<Event> &events);
};
