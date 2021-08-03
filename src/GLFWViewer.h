#pragma once

#include "Interfaces/IEmitter.h"
#include "Interfaces/IHandler.h"
#include "Interfaces/IViewer.h"

class GLFWwindow;

//! GLFWViewer uses the GLFW library to implement the IViewer, IEmitter, and
//! IHandler interfaces.
class GLFWViewer : public IViewer, public IEmitter, public IHandler {
  public:
    GLFWViewer();
    virtual ~GLFWViewer();

    virtual const char * GetClassName() const override { return "OpenXRVR"; }

    // ------------------------------------------------------------------------
    // IViewer interface.
    // ------------------------------------------------------------------------
    virtual bool Init(const ion::math::Vector2i &size);
    virtual void SetSize(const ion::math::Vector2i &new_size) override;
    virtual ion::math::Vector2i GetSize() const override;
    virtual void Render(IScene &scene, IRenderer &renderer) override;

    // ------------------------------------------------------------------------
    // IEmitter interface.
    // ------------------------------------------------------------------------
    virtual void EmitEvents(std::vector<Event> &events) override;

    // ------------------------------------------------------------------------
    // IHandler interface.
    // ------------------------------------------------------------------------
    virtual bool HandleEvent(const Event &event) override;

  private:
    GLFWwindow *window_ = nullptr;

    //! Events created by GLFW callbacks to add the next time EmitEvents() is
    //! called.
    std::vector<Event> pending_events_;

    //! Processes a key press or release.
    void ProcessKey_(int key, int action, int mods);

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
};
