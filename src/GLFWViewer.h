#pragma once

#include "Interfaces/IEmitter.h"
#include "Interfaces/IHandler.h"
#include "Interfaces/IViewer.h"
#include "Math/Types.h"
#include "SG/Typedefs.h"

class GLFWwindow;

//! GLFWViewer uses the GLFW library to implement the IViewer, IEmitter, and
//! IHandler interfaces.
class GLFWViewer : public IViewer, public IEmitter, public IHandler {
  public:
    GLFWViewer();
    virtual ~GLFWViewer();

    virtual const char * GetClassName() const override { return "GLFWViewer"; }

    //! Initializes the viewer with the given size and WindowCamera to use for
    //! computing a Frustum. Returns false if anything fails.
    bool Init(const Vector2i &size, const SG::WindowCameraPtr &camera);

    //! Returns the Frustum computed for the latest render.
    const Frustum & GetFrustum() const { return frustum_; }

    // ------------------------------------------------------------------------
    // IViewer interface.
    // ------------------------------------------------------------------------
    virtual void Render(const SG::Scene &scene, IRenderer &renderer);

    // ------------------------------------------------------------------------
    // IEmitter interface.
    // ------------------------------------------------------------------------
    virtual void EmitEvents(std::vector<Event> &events) override;

    // ------------------------------------------------------------------------
    // IHandler interface.
    // ------------------------------------------------------------------------
    virtual bool HandleEvent(const Event &event) override;

    // ------------------------------------------------------------------------
    // Other functions.
    // ------------------------------------------------------------------------

    //! Sets a flag indicating whether to wait for events or to poll events
    //! continuously. The default is to wait.
    void SetPollEventsFlag(bool should_poll) {
        should_poll_events_ = should_poll;
    }

  private:
    GLFWwindow *window_ = nullptr;

    //! Stores the camera used to set up the Frustum.
    SG::WindowCameraPtr camera_;

    //! Stores the current Frustum for the viewer.
    Frustum frustum_;

    //! Events created by GLFW callbacks to add the next time EmitEvents() is
    //! called.
    std::vector<Event> pending_events_;

    //! Whether to poll for events vs. wait for them. The default is false.
    bool should_poll_events_ = false;

    //! Updates the Frustum based on the current state.
    void UpdateFrustum_();

    //! Returns the current size of the window.
    Vector2i GetSize_() const;

    //! Processes a window resize to the given size.
    void ProcessResize_(int width, int height);

    //! Processes a key press or release.
    void ProcessKey_(int key, int action, int mods);

    //! Processes a mouse button press or release.
    void ProcessButton_(int button, int action, int mods);

    //! Processes a change in cursor position.
    void ProcessCursor_(double xpos, double ypos);

    //! Processes a scroll action.
    void ProcessScroll_(double xoffset, double yoffset);

    //! Stores the given cursor position in an Event.
    void StoreCursorPos_(double xpos, double ypos, Event &event);

    //! Retrieves the GLFWViewer instance stored as user data in the given
    //! window.
    static GLFWViewer & GetInstance_(GLFWwindow *window);

    //! GLFW error callback.
    static void ErrorCallback_(int error, const char *description);

    //! GLFW window resize callback.
    static void ResizeCallback_(GLFWwindow *window, int width, int height) {
        GetInstance_(window).ProcessResize_(width, height);
    }

    //! GLFW keyboard callback.
    static void KeyCallback_(GLFWwindow *window, int key,
                             int scancode, int action, int mods) {
        GetInstance_(window).ProcessKey_(key, action, mods);
    }

    //! GLFW mouse button callback.
    static void ButtonCallback_(GLFWwindow *window, int button,
                                int action, int mods) {
        GetInstance_(window).ProcessButton_(button, action, mods);
    }

    //! GLFW cursor position callback.
    static void CursorCallback_(GLFWwindow *window, double xpos, double ypos) {
        GetInstance_(window).ProcessCursor_(xpos, ypos);
    }

    //! GLFW scroll callback.
    static void ScrollCallback_(GLFWwindow *window,
                                double xoffset, double yoffset) {
        GetInstance_(window).ProcessScroll_(xoffset, yoffset);
    }
};
