#include "GLFWViewer.h"

#include <algorithm>

#include <GLFW/glfw3.h>

// This adds access to native types in glfw3.
#include "Viewers/ViewerContext.h"

#include "Event.h"
#include "Items/Controller.h"
#include "Math/Linear.h"
#include "Renderer.h"
#include "SG/WindowCamera.h"
#include "Util/Assert.h"

#include <cctype>

// ----------------------------------------------------------------------------
// Static helper functions.
// ----------------------------------------------------------------------------

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

/// Sets the key_name in the event based on the given key. This handles shifted
/// characters.
static void SetKeyName_(int key, Event &event) {
    const char *name = glfwGetKeyName(key, 0);

    // Handle special cases that GLFW does not for some reason.
    if (! name) {
        switch (key) {
          case GLFW_KEY_F1:        name = "F1";        break;
          case GLFW_KEY_F2:        name = "F2";        break;
          case GLFW_KEY_F3:        name = "F3";        break;
          case GLFW_KEY_F4:        name = "F4";        break;
          case GLFW_KEY_F5:        name = "F5";        break;
          case GLFW_KEY_F6:        name = "F6";        break;
          case GLFW_KEY_F7:        name = "F7";        break;
          case GLFW_KEY_F8:        name = "F8";        break;
          case GLFW_KEY_F9:        name = "F9";        break;
          case GLFW_KEY_F10:       name = "F10";       break;
          case GLFW_KEY_F11:       name = "F11";       break;
          case GLFW_KEY_F12:       name = "F12";       break;
          case GLFW_KEY_DOWN:      name = "Down";      break;
          case GLFW_KEY_ENTER:     name = "Enter";     break;
          case GLFW_KEY_ESCAPE:    name = "Escape";    break;
          case GLFW_KEY_LEFT:      name = "Left";      break;
          case GLFW_KEY_RIGHT:     name = "Right";     break;
          case GLFW_KEY_TAB:       name = "Tab";       break;
          case GLFW_KEY_UP:        name = "Up";        break;
          case GLFW_KEY_BACKSPACE: name = "Backspace"; break;
          case GLFW_KEY_SPACE:     name = " ";         break;

          default:
            // TODO: Add other required but unknown keys.
            std::cerr << "*** Unhandled key " << key << "\n";
            name = "UNKNOWN";
            break;
        }
    }

    event.key_name = name;

    // Handle shifted single keys.
    const bool is_shifted = event.modifiers.Has(Event::ModifierKey::kShift);
    if (is_shifted && event.key_name.size() == 1U) {
        const char c = event.key_name[0];
        char new_c = 0;
        if (c >= 'a' && c <= 'z') {
            new_c = std::toupper(c);
        }
        else {
            // Handle other special characters.
            switch (c) {
              case ',':  new_c = '<'; break;
              case '-':  new_c = '_'; break;
              case '.':  new_c = '>'; break;
              case '/':  new_c = '?'; break;
              case '0':  new_c = ')'; break;
              case '1':  new_c = '!'; break;
              case '2':  new_c = '@'; break;
              case '3':  new_c = '#'; break;
              case '4':  new_c = '$'; break;
              case '5':  new_c = '%'; break;
              case '6':  new_c = '^'; break;
              case '7':  new_c = '&'; break;
              case '8':  new_c = '*'; break;
              case '9':  new_c = '('; break;
              case ';':  new_c = ':'; break;
              case '=':  new_c = '+'; break;
              case '[':  new_c = '{'; break;
              case '\'': new_c = '"'; break;
              case '\\': new_c = '|'; break;
              case ']':  new_c = '}'; break;
              case '`':  new_c = '~'; break;
              default:                break;
            }
        }
        if (new_c) {
            event.key_name[0] = new_c;
            event.modifiers.Reset(Event::ModifierKey::kShift);
        }
    }
}

/// Creates and returns an Event instance representing a key press or release.
static Event GetKeyEvent_(bool is_press, int key, int mods) {
    Event event;
    event.device = Event::Device::kKeyboard;
    event.flags.Set(is_press ? Event::Flag::kKeyPress :
                    Event::Flag::kKeyRelease);

    // Store the modifiers first.
    if (mods & GLFW_MOD_SHIFT)
        event.modifiers.Set(Event::ModifierKey::kShift);
    if (mods & GLFW_MOD_CONTROL)
        event.modifiers.Set(Event::ModifierKey::kControl);
    if (mods & GLFW_MOD_ALT)
        event.modifiers.Set(Event::ModifierKey::kAlt);

    // Set the name of the key.
    SetKeyName_(key, event);

    return event;
}

// ----------------------------------------------------------------------------
// GLFWViewer implementation.
// ----------------------------------------------------------------------------

GLFWViewer::GLFWViewer() {
}

GLFWViewer::~GLFWViewer() {
    if (window_)
        glfwDestroyWindow(window_);
    glfwTerminate();
}

bool GLFWViewer::Init(const Vector2i &size) {
    ASSERT(! window_);

    if (! glfwInit()) {
        std::cerr << "*** GLFW initialization failed!\n";
        return false;
    }

    glfwSetErrorCallback(ErrorCallback_);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_SAMPLES, 16);
    window_ = glfwCreateWindow(size[0], size[1], "GLFW Window",
                               nullptr, nullptr);
    if (! window_) {
        std::cerr << "*** GLFW window creation failed!\n";
        return false;
    }
    glfwSetWindowPos(window_, 600, 100);

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

ViewerContext GLFWViewer::GetViewerContext() const {
    ASSERT(window_);
    ViewerContext vc;
#if defined(ION_PLATFORM_LINUX)
    vc.display  = glfwGetX11Display();
    vc.context  = glXGetCurrentContext();
    vc.drawable = glXGetCurrentDrawable();
#elif defined(ION_PLATFORM_WINDOWS)
    vc.dc   = GetDC(glfwGetWin32Window(window_));
    vc.glrc = glfwGetWGLContext(window_);
#endif
    return vc;
}

void GLFWViewer::Render(const SG::Scene &scene, Renderer &renderer) {
    UpdateFrustum_();

    Vector3f l_pos;
    Vector3f r_pos;
    if (l_controller_ && r_controller_) {
        // Move the controllers forward so they are visible in the window.
        const Vector3f kControllerOffset(0, 0, -10.5f);
        l_pos = l_controller_->GetTranslation();
        r_pos = r_controller_->GetTranslation();
        l_controller_->SetTranslation(l_pos + kControllerOffset);
        r_controller_->SetTranslation(r_pos + kControllerOffset);
    }

    glfwMakeContextCurrent(window_);
    renderer.RenderScene(scene, frustum_);
    glfwSwapBuffers(window_);

    if (l_controller_ && r_controller_) {
        // Restore the controller positions.
        l_controller_->SetTranslation(l_pos);
        r_controller_->SetTranslation(r_pos);
    }
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
    }
    // Add pending events.
    events.insert(events.end(), pending_events_.begin(), pending_events_.end());
    pending_events_.clear();
}

void GLFWViewer::FlushPendingEvents() {
    pending_events_.clear();
}

void GLFWViewer::UpdateFrustum_() {
    ASSERT(camera_);
    frustum_.viewport     = Viewport::BuildWithSize(Point2i(0, 0), GetSize_());
    frustum_.position     = camera_->GetPosition();
    frustum_.position[1] += camera_->GetHeight();
    frustum_.orientation  = camera_->GetOrientation();
    frustum_.pnear        = camera_->GetNear();
    frustum_.pfar         = camera_->GetFar();

    // Create a symmetric FOV.
    frustum_.SetSymmetricFOV(camera_->GetFOV(),
                             GetAspectRatio(frustum_.viewport));
}

Vector2i GLFWViewer::GetSize_() const {
    ASSERT(window_);
    int width, height;
    glfwGetWindowSize(window_, &width, &height);
    return Vector2i(width, height);
}

void GLFWViewer::ProcessChar_(unsigned int codepoint) {
    // TODO: This is unused for now. Remove if never needed.
}

void GLFWViewer::ProcessKey_(int key, int action, int mods) {
    // Keep track of shift press/release separately for alternate mode. Cannot
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
    // Normalize the position into the (0,1) range with (0,0) at the
    // lower-left. GLFW puts (0,0) at the upper-left.
    const Vector2i size = GetSize_();
    Point2f norm_pos(xpos / size[0], 1.0f - ypos / size[1]);

    event.flags.Set(Event::Flag::kPosition2D);
    event.position2D = norm_pos;
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
