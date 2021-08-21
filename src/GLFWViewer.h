#pragma once

#include <ion/math/vector.h>

#include "Interfaces/IEmitter.h"
#include "Interfaces/IHandler.h"
#include "Interfaces/IViewer.h"
#include "View.h"

class GLFWwindow;
class Scene;

//! GLFWViewer uses the GLFW library to implement the IViewer, IEmitter, and
//! IHandler interfaces.
class GLFWViewer : public IViewer, public IEmitter, public IHandler {
  public:
    GLFWViewer();
    virtual ~GLFWViewer();

    virtual const char * GetClassName() const override { return "GLFWViewer"; }

    // ------------------------------------------------------------------------
    // IViewer interface.
    // ------------------------------------------------------------------------
    virtual bool Init(const ion::math::Vector2i &size);
    virtual void SetSize(const ion::math::Vector2i &new_size) override;
    virtual void Render(IRenderer &renderer) override;
    virtual View & GetView() override { return view_; };

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

    //! Stores the current View for the viewer.
    View view_;

    //! Events created by GLFW callbacks to add the next time EmitEvents() is
    //! called.
    std::vector<Event> pending_events_;

    //! Whether to poll for events vs. wait for them. The default is false.
    bool should_poll_events_ = false;

    //! Updates the viewport in the View based on the current window size.
    void UpdateViewport_();

    //! Returns the current size of the window.
    ion::math::Vector2i GetSize_() const;

    //! Processes a key press or release.
    void ProcessKey_(int key, int action, int mods);

    //! Processes a mouse button press or release.
    void ProcessButton_(int button, int action, int mods);

    //! Processes a change in cursor position.
    void ProcessCursor_(double xpos, double ypos);

    //! Stores the given cursor position in an Event.
    void StoreCursorPos_(double xpos, double ypos, Event &event);

    //! Retrieves the GLFWViewer instance stored as user data in the given
    //! window.
    static GLFWViewer & GetInstance_(GLFWwindow *window);

    //! GLFW error callback.
    static void ErrorCallback_(int error, const char *description);

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
};
