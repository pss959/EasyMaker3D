#include "Viewers/GLFWViewer.h"

#include <algorithm>

#include <GLFW/glfw3.h>

#include <ion/portgfx/callbackcontext.h>

// Allow access to native types in glfw3.
#ifdef ION_PLATFORM_WINDOWS
#  define GLFW_EXPOSE_NATIVE_WIN32
#  include <GLFW/glfw3native.h>
#endif

#include "Base/Event.h"
#include "Math/Linear.h"
#include "SG/WindowCamera.h"
#include "Util/Assert.h"
#include "Util/KLog.h"
#include "Util/Tuning.h"
#include "Viewers/Renderer.h"

// ----------------------------------------------------------------------------
// Static helper functions.
// ----------------------------------------------------------------------------

/// Stores modifiers in an Event.
static void AddModifiers_(int mods, Event &event) {
    if (mods & GLFW_MOD_SHIFT)
        event.modifiers.Set(Event::ModifierKey::kShift);
    if (mods & GLFW_MOD_CONTROL)
        event.modifiers.Set(Event::ModifierKey::kControl);
    if (mods & GLFW_MOD_ALT)
        event.modifiers.Set(Event::ModifierKey::kAlt);
}

/// Returns true if the given key event should be ignored.
static bool IgnoreKeyEvent_(int key) {
    switch (key) {
        // Ignore modifier press/release - they are merged in as modifiers.
      case GLFW_KEY_LEFT_SHIFT:
      case GLFW_KEY_LEFT_CONTROL:
      case GLFW_KEY_LEFT_ALT:
      case GLFW_KEY_LEFT_SUPER:
      case GLFW_KEY_RIGHT_SHIFT:
      case GLFW_KEY_RIGHT_CONTROL:
      case GLFW_KEY_RIGHT_ALT:
      case GLFW_KEY_RIGHT_SUPER:
        return true;

      default:
        return false;
    }
}

/// Returns a name for the given key from GLFW.
static const char * GetKeyName_(int key) {
    const char *name = nullptr;

    // Map special keys to names first.
    switch (key) {
      case GLFW_KEY_BACKSPACE:    name = "Backspace";   break;
      case GLFW_KEY_CAPS_LOCK:    name = "CapsLock";    break;
      case GLFW_KEY_DELETE:       name = "Delete";      break;
      case GLFW_KEY_DOWN:         name = "Down";        break;
      case GLFW_KEY_END:          name = "End";         break;
      case GLFW_KEY_ENTER:        name = "Enter";       break;
      case GLFW_KEY_ESCAPE:       name = "Escape";      break;
      case GLFW_KEY_F10:          name = "F10";         break;
      case GLFW_KEY_F11:          name = "F11";         break;
      case GLFW_KEY_F12:          name = "F12";         break;
      case GLFW_KEY_F13:          name = "F13";         break;
      case GLFW_KEY_F14:          name = "F14";         break;
      case GLFW_KEY_F15:          name = "F15";         break;
      case GLFW_KEY_F16:          name = "F16";         break;
      case GLFW_KEY_F17:          name = "F17";         break;
      case GLFW_KEY_F18:          name = "F18";         break;
      case GLFW_KEY_F19:          name = "F19";         break;
      case GLFW_KEY_F1:           name = "F1";          break;
      case GLFW_KEY_F20:          name = "F20";         break;
      case GLFW_KEY_F21:          name = "F21";         break;
      case GLFW_KEY_F22:          name = "F22";         break;
      case GLFW_KEY_F23:          name = "F23";         break;
      case GLFW_KEY_F24:          name = "F24";         break;
      case GLFW_KEY_F25:          name = "F25";         break;
      case GLFW_KEY_F2:           name = "F2";          break;
      case GLFW_KEY_F3:           name = "F3";          break;
      case GLFW_KEY_F4:           name = "F4";          break;
      case GLFW_KEY_F5:           name = "F5";          break;
      case GLFW_KEY_F6:           name = "F6";          break;
      case GLFW_KEY_F7:           name = "F7";          break;
      case GLFW_KEY_F8:           name = "F8";          break;
      case GLFW_KEY_F9:           name = "F9";          break;
      case GLFW_KEY_HOME:         name = "Home";        break;
      case GLFW_KEY_INSERT:       name = "Insert";      break;
      case GLFW_KEY_KP_0:         name = "KP0";         break;
      case GLFW_KEY_KP_1:         name = "KP1";         break;
      case GLFW_KEY_KP_2:         name = "KP2";         break;
      case GLFW_KEY_KP_3:         name = "KP3";         break;
      case GLFW_KEY_KP_4:         name = "KP4";         break;
      case GLFW_KEY_KP_5:         name = "KP5";         break;
      case GLFW_KEY_KP_6:         name = "KP6";         break;
      case GLFW_KEY_KP_7:         name = "KP7";         break;
      case GLFW_KEY_KP_8:         name = "KP8";         break;
      case GLFW_KEY_KP_9:         name = "KP9";         break;
      case GLFW_KEY_KP_ADD:       name = "KPAdd";       break;
      case GLFW_KEY_KP_DECIMAL:   name = "KPDecimal";   break;
      case GLFW_KEY_KP_DIVIDE:    name = "KPDivide";    break;
      case GLFW_KEY_KP_ENTER:     name = "KPEnter";     break;
      case GLFW_KEY_KP_EQUAL:     name = "KPEqual";     break;
      case GLFW_KEY_KP_MULTIPLY:  name = "KPMultiply";  break;
      case GLFW_KEY_KP_SUBTRACT:  name = "KPSubtract";  break;
      case GLFW_KEY_LEFT:         name = "Left";        break;
      case GLFW_KEY_MENU:         name = "Menu";        break;
      case GLFW_KEY_NUM_LOCK:     name = "NumLock";     break;
      case GLFW_KEY_PAGE_DOWN:    name = "PageDown";    break;
      case GLFW_KEY_PAGE_UP:      name = "PageUp";      break;
      case GLFW_KEY_PAUSE:        name = "Pause";       break;
      case GLFW_KEY_PRINT_SCREEN: name = "PrintScreen"; break;
      case GLFW_KEY_RIGHT:        name = "Right";       break;
      case GLFW_KEY_SCROLL_LOCK:  name = "ScrollLock";  break;
      case GLFW_KEY_SPACE:        name = "Space";       break;
      case GLFW_KEY_TAB:          name = "Tab";         break;
      case GLFW_KEY_UP:           name = "Up";          break;
      default:
        break;
    }

    // If not handled above, see if GLFW has a name for the key.
    if (! name) {
        name = glfwGetKeyName(key, 0);
        if (! name)
            name = "UNKNOWN";
    }
    return name;
}

/// Creates and returns an Event instance representing a key press or release.
static Event GetKeyEvent_(bool is_press, int key, int mods) {
    Event event;
    event.device = Event::Device::kKeyboard;
    event.flags.Set(is_press ? Event::Flag::kKeyPress :
                    Event::Flag::kKeyRelease);

    // Store the modifiers first.
    AddModifiers_(mods, event);

    // Set the name and text for the key.
    event.key_name = GetKeyName_(key);
    event.key_text = Event::BuildKeyText(event.modifiers, event.key_name);

    return event;
}

// ----------------------------------------------------------------------------
// GLFWViewer implementation.
// ----------------------------------------------------------------------------

GLFWViewer::GLFWViewer() : frustum_(new Frustum) {
}

GLFWViewer::~GLFWViewer() {
    if (window_)
        glfwDestroyWindow(window_);
    glfwTerminate();
}

bool GLFWViewer::Init(const Vector2i &size, bool fullscreen) {
    ASSERT(! window_);

#ifdef ION_PLATFORM_MAC
    // Do NOT change directory on Mac. Not sure why this is not the default.
    glfwInitHint(GLFW_COCOA_CHDIR_RESOURCES, GLFW_FALSE);
#endif

    if (! glfwInit()) {
        std::cerr << "*** GLFW initialization failed!\n";
        return false;
    }

    glfwSetErrorCallback(ErrorCallback_);

    glfwWindowHint(GLFW_OPENGL_PROFILE,         GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT,  GL_TRUE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,  3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,  3);
    glfwWindowHint(GLFW_SAMPLES,                16);
    const Str title = TK::kApplicationName + " " + TK::kVersionString;
    window_ = glfwCreateWindow(size[0], size[1], title.c_str(),
                               nullptr, nullptr);
    if (! window_) {
        std::cerr << "*** GLFW window creation failed!\n";
        return false;
    }

    // Set up Ion callbacks.
    InitIonCallbacks_();

    glfwSetWindowPos(window_, 600, 100);

    // Fullscreen is just a maximized window. This allows the user to
    // unmaximize it and move it around if they want. True fullscreen mode is
    // kind of annoying in that respect.
    if (fullscreen)
        glfwMaximizeWindow(window_);

    glfwSetCharCallback(window_,        CharCallback_);
    glfwSetKeyCallback(window_,         KeyCallback_);
    glfwSetMouseButtonCallback(window_, ButtonCallback_);
    glfwSetCursorPosCallback(window_,   CursorCallback_);
    glfwSetScrollCallback(window_,      ScrollCallback_);

    // Store this instance as user data so the static functions can access it.
    glfwSetWindowUserPointer(window_, this);

    glfwMakeContextCurrent(window_);

    return true;
}

void GLFWViewer::Render(const SG::Scene &scene, Renderer &renderer) {
    UpdateFrustum_();

    glfwMakeContextCurrent(window_);
    renderer.RenderScene(scene, *frustum_);
    glfwSwapBuffers(window_);
}

void GLFWViewer::EmitEvents(std::vector<Event> &events) {
    if (should_poll_events_)
        glfwPollEvents();
    else
        glfwWaitEvents();

    // Check for termination.
    if (glfwWindowShouldClose(window_)) {
        Event event;
        event.device = Event::Device::kMouse;
        event.flags.Set(Event::Flag::kExit);
        events.push_back(event);

        // Reset the flag; the event will end the app if the user wants it.
        glfwSetWindowShouldClose(window_, GLFW_FALSE);
    }

    // Add pending events.
    events.insert(events.end(), pending_events_.begin(), pending_events_.end());
    pending_events_.clear();

    if (events.size() > 1U)
        CompressEvents_(events);
}

void GLFWViewer::FlushPendingEvents() {
    pending_events_.clear();
}

void GLFWViewer::InitIonCallbacks_() {
    using ion::portgfx::CallbackContext;

    ASSERT(window_);

    auto init      = [&](){ return window_; };
    auto is_valid  = [&](){ return true; };
    auto get_addr  = [&](const char *name, unsigned int flags){
        return reinterpret_cast<void *>(glfwGetProcAddress(name));
    };
    auto swap_buf  = [&](){ glfwSwapBuffers(window_); };
    auto make_cur  = [&](CallbackContext::ContextID id){
        glfwMakeContextCurrent(reinterpret_cast<GLFWwindow *>(id));
        return true;
    };
    auto clear_cur = [&](){ glfwMakeContextCurrent(nullptr); };
    auto get_cur   = [&](){
        return reinterpret_cast<uintptr_t>(glfwGetCurrentContext());
    };

    CallbackContext::SetInitCB(init);
    CallbackContext::SetIsValidCB(is_valid);
    CallbackContext::SetGetProcAddressCB(get_addr);
    CallbackContext::SetSwapBuffersCB(swap_buf);
    CallbackContext::SetMakeCurrentCB(make_cur);
    CallbackContext::SetClearCurrentCB(clear_cur);
    CallbackContext::SetGetCurrentCB(get_cur);
}

void GLFWViewer::UpdateFrustum_() {
    ASSERT(camera_);
    auto &frustum = *frustum_;
    frustum.viewport     = Viewport::BuildWithSize(Point2i(0, 0),
                                                   GetWindowSizePixels_());
    frustum.position     = camera_->GetPosition();
    frustum.position[1] += camera_->GetHeight();
    frustum.orientation  = camera_->GetOrientation();
    frustum.pnear        = camera_->GetNear();
    frustum.pfar         = camera_->GetFar();

    // Create a symmetric FOV.
    frustum.SetSymmetricFOV(camera_->GetFOV(),
                            GetAspectRatio(frustum.viewport));
}

Vector2i GLFWViewer::GetWindowSizeScreenCoords_() const {
    ASSERT(window_);
    int width, height;
    glfwGetWindowSize(window_, &width, &height);
    return Vector2i(width, height);
}

Vector2i GLFWViewer::GetWindowSizePixels_() const {
    ASSERT(window_);
    int width, height;
    // Note: the GLFW window size is measured in screen coordinates, not
    // pixels. On some displays (ahem, Mac) these are not the same, so use the
    // framebuffer size here, which is always in pixels.
    glfwGetFramebufferSize(window_, &width, &height);
    return Vector2i(width, height);
}

void GLFWViewer::ProcessChar_(unsigned int codepoint) {
    /// \todo This is unused for now. Remove if never needed.
}

void GLFWViewer::ProcessKey_(int key, int action, int mods) {
    // Keep track of shift press/release separately for modified mode. Cannot
    // rely on mods to be set correctly when shift keys are pressed/released.
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_LEFT_SHIFT)
            is_l_shift_pressed_ = true;
        else if (key == GLFW_KEY_RIGHT_SHIFT)
            is_r_shift_pressed_ = true;
        else if (! IgnoreKeyEvent_(key))
            pending_events_.push_back(GetKeyEvent_(true, key, mods));
    }
    else if (action == GLFW_RELEASE) {
        if (key == GLFW_KEY_LEFT_SHIFT)
            is_l_shift_pressed_ = false;
        else if (key == GLFW_KEY_RIGHT_SHIFT)
            is_r_shift_pressed_ = false;
        if (! IgnoreKeyEvent_(key))
            pending_events_.push_back(GetKeyEvent_(false, key, mods));
    }
    else if (action == GLFW_REPEAT) {
        if (! IgnoreKeyEvent_(key)) {
            pending_events_.push_back(GetKeyEvent_(true,  key, mods));
            pending_events_.push_back(GetKeyEvent_(false, key, mods));
        }
    }
}

void GLFWViewer::ProcessButton_(int button, int action, int mods) {
    if (action == GLFW_PRESS || action == GLFW_RELEASE) {
        Event event;
        event.device = Event::Device::kMouse;
        event.flags.Set(action == GLFW_PRESS ? Event::Flag::kButtonPress :
                        Event::Flag::kButtonRelease);
        switch (button) {
          case GLFW_MOUSE_BUTTON_LEFT:
            event.button = Event::Button::kMouse1;
            break;
          case GLFW_MOUSE_BUTTON_MIDDLE:
            event.button = Event::Button::kMouse2;
            break;
          case GLFW_MOUSE_BUTTON_RIGHT:
            event.button = Event::Button::kMouse3;
            break;
          default:
            event.button = Event::Button::kOther;
            break;
        }

        // Add modifiers.
        AddModifiers_(mods, event);

        // Also store the current position.
        double xpos, ypos;
        glfwGetCursorPos(window_, &xpos, &ypos);
        StoreCursorPos_(xpos, ypos, event);

        pending_events_.push_back(event);
    }
}

void GLFWViewer::ProcessCursor_(double xpos, double ypos) {
    Event event;
    event.device = Event::Device::kMouse;
    StoreCursorPos_(xpos, ypos, event);
    pending_events_.push_back(event);
}

void GLFWViewer::ProcessScroll_(double xoffset, double yoffset) {
    // Mouse scroll wheel scrolls in Y.
    if (yoffset) {
        Event event;
        event.device = Event::Device::kMouse;
        event.flags.Set(Event::Flag::kPosition1D);
        event.position1D = static_cast<float>(yoffset);
        pending_events_.push_back(event);
    }
}

void GLFWViewer::StoreCursorPos_(double xpos, double ypos, Event &event) {
    // Do NOT do this if ignoring mouse motion.
    if (is_mouse_motion_enabled_) {
        // Normalize the position into the (0,1) range with (0,0) at the
        // lower-left. GLFW puts (0,0) at the upper-left.
        const Vector2i size = GetWindowSizeScreenCoords_();
        Point2f norm_pos(xpos / size[0], 1.0f - ypos / size[1]);

        event.flags.Set(Event::Flag::kPosition2D);
        event.position2D = norm_pos;
    }
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

GLFWViewer & GLFWViewer::GetInstance_(GLFWwindow *window) {
    ASSERT(window);
    GLFWViewer *instance =
        reinterpret_cast<GLFWViewer *>(glfwGetWindowUserPointer(window));
    ASSERT(instance);
    return *instance;
}

void GLFWViewer::ErrorCallback_(int error, const char *description) {
    std::cerr << "*** GLFW Error " << error << ": " << description << "\n";
}
