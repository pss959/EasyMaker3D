#pragma once

#include "Base/IEmitter.h"
#include "Base/Memory.h"
#include "Math/Types.h"
#include "Viewers/Viewer.h"

class GLFWwindow;

DECL_SHARED_PTR(Frustum);
DECL_SHARED_PTR(GLFWViewer);
namespace SG { DECL_SHARED_PTR(WindowCamera); }

/// GLFWViewer is a derived Viewer and IEmitter that uses the GLFW library
/// to do windowing and input.
///
/// \ingroup Viewers
class GLFWViewer : public Viewer, public IEmitter {
  public:
    GLFWViewer();
    virtual ~GLFWViewer();

    /// Initializes the viewer with the given size and fullscreen flag. Returns
    /// false if anything fails.
    bool Init(const Vector2i &size, bool fullscreen);

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
        should_poll_events_ = should_poll;
    }

    /// Returns true if either shift key is currenty pressed.
    bool IsShiftKeyPressed() const {
        return is_l_shift_pressed_ || is_r_shift_pressed_;
    }

    /// Returns the current size of the window in pixels, taking the fullscreen
    /// option into account.
    Vector2i GetWindowSize() const { return GetWindowSizePixels_(); }

    virtual void Render(const SG::Scene &scene, IRenderer &renderer);

    virtual void EmitEvents(std::vector<Event> &events) override;
    virtual void FlushPendingEvents() override;

    /// This can be used to temporarily ignore mouse motion events produced by
    /// GLFW. It is used by the snapimage application to avoid spurious events
    /// caused by accidental mouse motion during processing. They are enabled
    /// by default.
    void EnableMouseMotionEvents(bool enable) {
        is_mouse_motion_enabled_ = enable;
    }

  private:
    GLFWwindow *window_ = nullptr;

    /// Stores the camera used to set up the Frustum.
    SG::WindowCameraPtr camera_;

    /// Stores the current Frustum for the viewer.
    FrustumPtr frustum_;

    /// Events created by GLFW callbacks to add the next time EmitEvents() is
    /// called.
    std::vector<Event> pending_events_;

    /// Whether to poll for events vs. wait for them. The default is false.
    bool should_poll_events_ = false;

    /// Stores whether the left shift key is pressed.
    bool is_l_shift_pressed_ = false;
    /// Stores whether the right shift key is pressed.
    bool is_r_shift_pressed_ = false;

    /// Used for temporarily ignoring mouse motion.
    bool is_mouse_motion_enabled_ = true;

    /// Sets the callbacks in the ion::portgfx::CallbackContext to use GLFW
    /// functions.
    void InitIonCallbacks_();

    /// Updates the Frustum based on the current state.
    void UpdateFrustum_();

    /// Returns the current size of the window in screen coordinates.
    Vector2i GetWindowSizeScreenCoords_() const;

    /// Returns the current size of the window in pixels.
    Vector2i GetWindowSizePixels_() const;

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

    /// Compresses events in the given vector: multiple mouse motion events
    /// with no button presses or releases are compressed into the last one.
    void CompressEvents_(std::vector<Event> &events);

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
