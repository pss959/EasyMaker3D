//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Base/GLFWWindowSystem.h"

#include <GLFW/glfw3.h>

#include <ion/portgfx/callbackcontext.h>

// Allow access to native types in glfw3.
#ifdef ION_PLATFORM_WINDOWS
#  define GLFW_EXPOSE_NATIVE_WIN32
#  include <GLFW/glfw3native.h>
#endif

#include "Base/Event.h"
#include "Util/Assert.h"
#include "Util/General.h"
#include "Util/String.h"
#include "Util/Tuning.h"

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

GLFWWindowSystem::ErrorFunc GLFWWindowSystem::error_func_;

bool GLFWWindowSystem::Init(const ErrorFunc &error_func) {
    ASSERT(error_func);
    ASSERT(! window_);
    error_func_ = error_func;

#ifdef ION_PLATFORM_MAC
    // Do NOT change directory on Mac. Not sure why this is not the default.
    glfwInitHint(GLFW_COCOA_CHDIR_RESOURCES, GLFW_FALSE);
#endif

    if (! glfwInit()) {
        error_func_("GLFW initialization failed!");
        return false;
    }

    glfwSetErrorCallback(ErrorCallback_);

    return true;
}

void GLFWWindowSystem::Terminate() {
    if (window_)
        glfwDestroyWindow(window_);
    glfwTerminate();
}

bool GLFWWindowSystem::CreateMainWindow(const Vector2ui &size, const Str &title,
                                        bool offscreen) {
    glfwWindowHint(GLFW_OPENGL_PROFILE,         GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT,  GL_TRUE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,  3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,  3);
    glfwWindowHint(GLFW_SAMPLES,                TK::kNonVRSampleCount);

    if (offscreen)
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    window_ = glfwCreateWindow(size[0], size[1], title.c_str(),
                               nullptr, nullptr);
    if (! window_) {
        ASSERT(error_func_);
        error_func_("GLFW window creation failed!");
        return false;
    }

    // Set up Ion callbacks.
    InitIonCallbacks_();

    glfwSetKeyCallback(window_,         KeyCallback_);
    glfwSetMouseButtonCallback(window_, ButtonCallback_);
    glfwSetCursorPosCallback(window_,   CursorCallback_);
    glfwSetScrollCallback(window_,      ScrollCallback_);

    // Store this instance as user data so the static functions can access it.
    glfwSetWindowUserPointer(window_, this);

    glfwMakeContextCurrent(window_);

    return true;
}

void GLFWWindowSystem::SetWindowPosition(const Point2ui &pos) {
    ASSERT(window_);
    glfwSetWindowPos(window_, pos[0], pos[1]);
}

void GLFWWindowSystem::Maximize() {
    ASSERT(window_);

    glfwMaximizeWindow(window_);

#if WORK_AROUND_GLFW_BUG
    // The size of a hidden window apparently does NOT get updated when
    // maximized. To work around this, have to show the window temporarily to
    // update the size and hide it. No window will appear, but it glitches the
    // window manager briefly.
    if (! glfwGetWindowAttrib(window_, GLFW_VISIBLE)) {
        glfwShowWindow(window_);
        glfwHideWindow(window_);
    }
#endif
}

Vector2ui GLFWWindowSystem::GetWindowSize() {
    ASSERT(window_);
    int width, height;
    glfwGetWindowSize(window_, &width, &height);
    return Vector2ui(width, height);
}

Vector2ui GLFWWindowSystem::GetFramebufferSize() {
    ASSERT(window_);
    int width, height;
    glfwGetFramebufferSize(window_, &width, &height);
    return Vector2ui(width, height);
}

void GLFWWindowSystem::PreRender() {
    ASSERT(window_);
    glfwMakeContextCurrent(window_);
}

void GLFWWindowSystem::PostRender() {
    ASSERT(window_);
    glfwSwapBuffers(window_);
}

bool GLFWWindowSystem::WasWindowClosed() {
    if (glfwWindowShouldClose(window_)) {
        // Reset the flag; the event will end the app if the user wants it.
        glfwSetWindowShouldClose(window_, GLFW_FALSE);
        return true;
    }
    return false;
}

void GLFWWindowSystem::RetrieveEvents(const EventOptions &options,
                                      std::vector<Event> &events) {
    is_mouse_motion_enabled_ = ! options.ignore_mouse_motion;

    if (options.wait_for_events)
        glfwWaitEvents();
    else
        glfwPollEvents();

    // Add pending events.
    Util::AppendVector(pending_events_, events);
    pending_events_.clear();
}

void GLFWWindowSystem::FlushPendingEvents() {
    pending_events_.clear();
}

bool GLFWWindowSystem::IsShiftKeyPressed() {
    return is_l_shift_pressed_ || is_r_shift_pressed_;
}

void GLFWWindowSystem::InitIonCallbacks_() {
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

GLFWWindowSystem & GLFWWindowSystem::GetInstance_(GLFWwindow *window) {
    ASSERT(window);
    auto *instance =
        reinterpret_cast<GLFWWindowSystem *>(glfwGetWindowUserPointer(window));
    ASSERT(instance);
    return *instance;
}

void GLFWWindowSystem::ErrorCallback_(int error, const char *description) {
    ASSERT(error_func_);
    error_func_("GLFW Error " + Util::ToString(error) + ": " + description);
}


void GLFWWindowSystem::KeyCallback_(GLFWwindow *window, int key,
                                    int scancode, int action, int mods) {
    GetInstance_(window).ProcessKey_(key, action, mods);
}

void GLFWWindowSystem::ButtonCallback_(GLFWwindow *window, int button,
                                       int action, int mods) {
    GetInstance_(window).ProcessButton_(button, action, mods);
}

void GLFWWindowSystem::CursorCallback_(GLFWwindow *window,
                                       double xpos, double ypos) {
    GetInstance_(window).ProcessCursor_(xpos, ypos);
}

void GLFWWindowSystem::ScrollCallback_(GLFWwindow *window,
                                       double xoffset, double yoffset) {
    GetInstance_(window).ProcessScroll_(xoffset, yoffset);
}

void GLFWWindowSystem::ProcessKey_(int key, int action, int mods) {
    // Keep track of shift press/release separately for modified mode. Cannot
    // rely on mods parameter to be set correctly when shift keys are
    // pressed/released.
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

void GLFWWindowSystem::ProcessButton_(int button, int action, int mods) {
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
        StoreCurrentCursorPos_(event);

        pending_events_.push_back(event);
    }
}

void GLFWWindowSystem::ProcessCursor_(double xpos, double ypos) {
    if (is_mouse_motion_enabled_) {
        Event event;
        event.device = Event::Device::kMouse;
        StoreCursorPos_(xpos, ypos, event);
        pending_events_.push_back(event);
    }
}

void GLFWWindowSystem::ProcessScroll_(double xoffset, double yoffset) {
    // Mouse scroll wheel scrolls in Y.
    if (yoffset) {
        Event event;
        event.device = Event::Device::kMouse;
        event.flags.Set(Event::Flag::kPosition1D);
        event.position1D = static_cast<float>(yoffset);

        // Store the current cursor position in the event so it can be
        // correlated to an object that receives the scroll event.
        StoreCurrentCursorPos_(event);

        pending_events_.push_back(event);
    }
}

void GLFWWindowSystem::StoreCurrentCursorPos_(Event &event) {
    double xpos, ypos;
    glfwGetCursorPos(window_, &xpos, &ypos);
    StoreCursorPos_(xpos, ypos, event);
}

void GLFWWindowSystem::StoreCursorPos_(double xpos, double ypos, Event &event) {
    // Normalize the position into the (0,1) range with (0,0) at the
    // lower-left. GLFW puts (0,0) at the upper-left.
    const Vector2ui size = GetWindowSize();
    const Point2f norm_pos(xpos / size[0], 1.0f - ypos / size[1]);

    event.flags.Set(Event::Flag::kPosition2D);
    event.position2D = norm_pos;
}
