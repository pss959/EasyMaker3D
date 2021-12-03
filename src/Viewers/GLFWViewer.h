#pragma once

#include <memory>

#include "Items/Controller.h"
#include "Math/Types.h"
#include "SG/Typedefs.h"
#include "Viewers/Viewer.h"
#include "Viewers/ViewerContext.h"

class GLFWwindow;

/// GLFWViewer is a derived Viewer that uses the GLFW library to do windowing
/// and input.
class GLFWViewer : public Viewer {
  public:
    GLFWViewer();
    virtual ~GLFWViewer();

    /// Initializes the viewer with the given size. Returns false if anything
    /// fails.
    bool Init(const Vector2i &size);

    /// Returns a ViewerContext filled in with platform-dependent values. This
    /// assumes Init() has completed successfully.
    ViewerContext GetViewerContext() const;

    /// Sets the WindowCamera to update.
    void SetCamera(const SG::WindowCameraPtr &camera) {
        camera_ = camera;
        UpdateFrustum_();
    }

    /// Sets the left and right controllers so that they can be moved to appear
    /// in the window when in VR.
    void SetControllers(const ControllerPtr &l_controller,
                        const ControllerPtr &r_controller) {
        l_controller_ = l_controller;
        r_controller_ = r_controller;
    }

    /// Returns the Frustum computed for the latest render.
    const Frustum & GetFrustum() const { return frustum_; }

    /// Sets a flag indicating whether to wait for events or to poll events
    /// continuously. The default is to wait.
    void SetPollEventsFlag(bool should_poll) {
        should_poll_events_ = should_poll;
    }

    virtual void Render(const SG::Scene &scene, Renderer &renderer);
    virtual void EmitEvents(std::vector<Event> &events) override;
    virtual void FlushPendingEvents() override;

  private:
    GLFWwindow *window_ = nullptr;

    /// Stores the camera used to set up the Frustum.
    SG::WindowCameraPtr camera_;

    ControllerPtr l_controller_;  ///< Left controller.
    ControllerPtr r_controller_;  ///< Right controller.

    /// Stores the current Frustum for the viewer.
    Frustum frustum_;

    /// Events created by GLFW callbacks to add the next time EmitEvents() is
    /// called.
    std::vector<Event> pending_events_;

    /// Whether to poll for events vs. wait for them. The default is false.
    bool should_poll_events_ = false;

    /// Updates the Frustum based on the current state.
    void UpdateFrustum_();

    /// Returns the current size of the window.
    Vector2i GetSize_() const;

    /// Processes a text character.
    void ProcessChar_(unsigned int codepoint);

    /// Processes a key press or release.
    void ProcessKey_(int key, int action, int mods);

    /// Processes a mouse button press or release.
    void ProcessButton_(int button, int action, int mods);

    /// Processes a change in cursor position.
    void ProcessCursor_(double xpos, double ypos);

    /// Processes a scroll action.
    void ProcessScroll_(double xoffset, double yoffset);

    /// Stores the given cursor position in an Event.
    void StoreCursorPos_(double xpos, double ypos, Event &event);

    /// Retrieves the GLFWViewer instance stored as user data in the given
    /// window.
    static GLFWViewer & GetInstance_(GLFWwindow *window);

    /// GLFW error callback.
    static void ErrorCallback_(int error, const char *description);

    /// GLFW keyboard character callback.
    static void CharCallback_(GLFWwindow *window, unsigned int codepoint) {
        GetInstance_(window).ProcessChar_(codepoint);
    }

    /// GLFW keyboard callback.
    static void KeyCallback_(GLFWwindow *window, int key,
                             int scancode, int action, int mods) {
        GetInstance_(window).ProcessKey_(key, action, mods);
    }

    /// GLFW mouse button callback.
    static void ButtonCallback_(GLFWwindow *window, int button,
                                int action, int mods) {
        GetInstance_(window).ProcessButton_(button, action, mods);
    }

    /// GLFW cursor position callback.
    static void CursorCallback_(GLFWwindow *window, double xpos, double ypos) {
        GetInstance_(window).ProcessCursor_(xpos, ypos);
    }

    /// GLFW scroll callback.
    static void ScrollCallback_(GLFWwindow *window,
                                double xoffset, double yoffset) {
        GetInstance_(window).ProcessScroll_(xoffset, yoffset);
    }
};

typedef std::shared_ptr<GLFWViewer> GLFWViewerPtr;
