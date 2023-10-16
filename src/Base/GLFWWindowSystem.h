#pragma once

#include "Base/IWindowSystem.h"

class GLFWwindow;

/// GLFWWindowSystem is a derived IWindowSystem class that uses the GLFW
/// library to manage windows and interaction.
///
/// \ingroup Base
class GLFWWindowSystem : public IWindowSystem {
  public:
    virtual bool Init(const ErrorFunc &error_func) override;
    virtual void Terminate() override;
    virtual bool CreateWindow(const Vector2i &size, const Str &title) override;
    virtual void SetWindowPosition(const Vector2i &pos) override;
    virtual void SetFullScreen() override;
    virtual Vector2i GetWindowSize() override;
    virtual Vector2i GetFramebufferSize() override;
    virtual void PreRender() override;
    virtual void PostRender() override;
    virtual bool WasWindowClosed() override;
    virtual void RetrieveEvents(const EventOptions &options,
                                std::vector<Event> &events) override;
    virtual void FlushPendingEvents() override;
    virtual bool IsShiftKeyPressed() override;

  private:
    /// Error function passed to Init(). This has to be static because the
    /// GLFW error callback contains nothing to access an instance from.
    static ErrorFunc error_func_;

    /// Stores the GLFWwindow created by CreateWindow().
    GLFWwindow *window_ = nullptr;

    /// Stores whether the left shift key is pressed.
    bool is_l_shift_pressed_ = false;
    /// Stores whether the right shift key is pressed.
    bool is_r_shift_pressed_ = false;

    /// Used for temporarily ignoring mouse motion.
    bool is_mouse_motion_enabled_ = true;

    /// Events created by GLFW callbacks to add the next time EmitEvents() is
    /// called.
    std::vector<Event> pending_events_;

    /// Sets the callbacks in the ion::portgfx::CallbackContext to use GLFW
    /// functions.
    void InitIonCallbacks_();

    /// Retrieves the GLFWWindowSystem instance stored as user data in the
    /// given GLFWwindow.
    static GLFWWindowSystem & GetInstance_(GLFWwindow *window);

    /// \name GLFW callbacks.
    /// Note that these need to be static functions because they have to be
    /// convertible to function pointers.
    ///@{

    /// GLFW error callback.
    static void ErrorCallback_(int error, const char *description);

    /// GLFW keyboard callback.
    static void KeyCallback_(GLFWwindow *window, int key,
                             int scancode, int action, int mods);

    /// GLFW mouse button callback.
    static void ButtonCallback_(GLFWwindow *window, int button,
                                int action, int mods);

    /// GLFW cursor position callback.
    static void CursorCallback_(GLFWwindow *window, double xpos, double ypos);

    /// GLFW scroll callback.
    static void ScrollCallback_(GLFWwindow *window,
                                double xoffset, double yoffset);

    ///@}

    /// \name Event processing.
    ///@{

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

    ///@}
};
