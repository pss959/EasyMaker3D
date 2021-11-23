#include "GLFWViewer.h"

#include <GLFW/glfw3.h>

#include "Assert.h"
#include "Event.h"
#include "Math/Linear.h"
#include "Renderer.h"
#include "SG/WindowCamera.h"

// ----------------------------------------------------------------------------
// Static helper functions.
// ----------------------------------------------------------------------------

/// Creates and returns an Event instance representing a key press or release.
static Event GetKeyEvent_(bool is_press, int key, int mods) {
    Event event;
    event.device = Event::Device::kKeyboard;
    event.flags.Set(is_press ? Event::Flag::kKeyPress :
                    Event::Flag::kKeyRelease);

    // Build the detail string.
    event.key_string = "";
    if (mods & GLFW_MOD_SHIFT)
        event.key_string += "<Shift>";
    if (mods & GLFW_MOD_CONTROL)
        event.key_string += "<Ctrl>";
    if (mods & GLFW_MOD_ALT)
        event.key_string += "<Alt>";
    const char *name = glfwGetKeyName(key, 0);
    if (! name) {
        // Handle special cases that GLFW does not for some reason.
        switch (key) {
          case GLFW_KEY_DOWN:      name = "Down";      break;
          case GLFW_KEY_ENTER:     name = "Enter";     break;
          case GLFW_KEY_ESCAPE:    name = "Escape";    break;
          case GLFW_KEY_LEFT:      name = "Left";      break;
          case GLFW_KEY_RIGHT:     name = "Right";     break;
          case GLFW_KEY_TAB:       name = "Tab";       break;
          case GLFW_KEY_UP:        name = "Up";        break;
          case GLFW_KEY_BACKSPACE: name = "Backspace"; break;
          default:
            // TODO: Add other required but unknown keys.
            std::cerr << "*** Unhandled key " << key << "\n";
            name = "UNKNOWN";
            break;
        }
    }
    event.key_string += name;

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

void GLFWViewer::Render(const SG::Scene &scene, Renderer &renderer) {
    ASSERT(l_controller_);
    ASSERT(r_controller_);

    UpdateFrustum_();

    // Move the controllers forward so they are visible in the window.
    const Vector3f kControllerOffset(0, 0, -10.5f);
    const Vector3f l_pos = l_controller_->GetTranslation();
    const Vector3f r_pos = r_controller_->GetTranslation();
    l_controller_->SetTranslation(l_pos + kControllerOffset);
    r_controller_->SetTranslation(r_pos + kControllerOffset);

    glfwMakeContextCurrent(window_);
    renderer.RenderScene(scene, frustum_);
    glfwSwapBuffers(window_);

    // Restore the controller positions.
    l_controller_->SetTranslation(l_pos);
    r_controller_->SetTranslation(r_pos);
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
    frustum_.near         = camera_->GetNear();
    frustum_.far          = camera_->GetFar();

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
    // XXXX  This is unused for now...
    // std::cerr << "XXXX Got codepoint " << codepoint << "\n";
}

void GLFWViewer::ProcessKey_(int key, int action, int mods) {
    if (action == GLFW_PRESS) {
        pending_events_.push_back(GetKeyEvent_(true, key, mods));
    }
    else if (action == GLFW_RELEASE) {
        pending_events_.push_back(GetKeyEvent_(false, key, mods));
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
