#include "GLFWViewer.h"

#include <assert.h>

#include <GLFW/glfw3.h>

#include <ion/math/range.h>

#include "Event.h"
#include "Interfaces/IRenderer.h"

using ion::math::Point2i;
using ion::math::Range2i;
using ion::math::Vector2i;

// ----------------------------------------------------------------------------
// Static helper functions.
// ----------------------------------------------------------------------------

//! Creates and returns an Event instance representing a key press or release.
static Event GetKeyEvent_(bool is_press, int key, int mods) {
    Event ev;
    ev.flags = static_cast<uint32_t>(
        is_press ? Event::Flag::kKeyPress : Event::Flag::kKeyRelease);

    // Build the detail string.
    ev.detail_string = "";
    if (mods & GLFW_MOD_SHIFT)
        ev.detail_string += "<Shift>";
    if (mods & GLFW_MOD_CONTROL)
        ev.detail_string += "<Ctrl>";
    if (mods & GLFW_MOD_ALT)
        ev.detail_string += "<Alt>";
    const char *name = glfwGetKeyName(key, 0);
    if (! name) {
        // Handle special cases that GLFW does not for some reason.
        if (key == GLFW_KEY_ESCAPE)
            name = "Escape";
        else {
            std::cerr << "XXXX No name for key " << key << "\n";
            name = "UNKNOWN";
        }
    }
    ev.detail_string += name;

    return ev;
}

// ----------------------------------------------------------------------------
// GLFWViewer implementation.
// ----------------------------------------------------------------------------

GLFWViewer::GLFWViewer() {
}

GLFWViewer::~GLFWViewer() {
    assert(window_);
    glfwDestroyWindow(window_);
    glfwTerminate();
}

bool GLFWViewer::Init(const ion::math::Vector2i &size) {
    assert(! window_);

    if (! glfwInit()) {
        std::cerr << "*** GLFW initialization failed!\n";
        return false;
    }

    glfwSetErrorCallback(ErrorCallback_);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    window_ = glfwCreateWindow(size[0], size[1], "GLFW Window",
                               nullptr, nullptr);
    if (! window_) {
        std::cerr << "*** GLFW window creation failed!\n";
        return false;
    }

    glfwSetKeyCallback(window_, KeyCallback_);

    // Store this instance as user data so the static functions can access it.
    glfwSetWindowUserPointer(window_, this);

    glfwMakeContextCurrent(window_);

    return true;
}

void GLFWViewer::SetSize(const Vector2i &new_size) {
    assert(window_);
    glfwSetWindowSize(window_, new_size[0], new_size[1]);
}

Vector2i GLFWViewer::GetSize() const {
    assert(window_);
    int width, height;
    glfwGetWindowSize(window_, &width, &height);
    return Vector2i(width, height);
}

void GLFWViewer::Render(IScene &scene, IRenderer &renderer) {
    assert(window_);
    int width, height;
    glfwGetWindowSize(window_, &width, &height);

    Range2i viewport = Range2i::BuildWithSize(Point2i(0, 0),
                                              Vector2i(width, height));
    glfwMakeContextCurrent(window_);
    renderer.RenderScene(scene, viewport);
    glfwSwapBuffers(window_);
}

void GLFWViewer::EmitEvents(std::vector<Event> &events) {
    glfwPollEvents();

    // Check for termination.
    if (glfwWindowShouldClose(window_)) {
        Event ev;
        ev.flags = static_cast<uint32_t>(Event::Flag::kExit);
        events.push_back(ev);
    }
    // Add pending events.
    events.insert(events.end(), pending_events_.begin(), pending_events_.end());
    pending_events_.clear();
}

bool GLFWViewer::HandleEvent(const Event &event) {
    if (event.HasFlag(Event::Flag::kKeyPress)) {
        if (event.detail_string == "Escape") {
            glfwSetWindowShouldClose(window_, GLFW_TRUE);
            return true;
        }
    }
    return false;
}

void GLFWViewer::ProcessKey_(int key, int action, int mods) {
    if (action == GLFW_PRESS) {
        pending_events_.push_back(GetKeyEvent_(true, key, mods));
    }
    else if (action == GLFW_RELEASE) {
        pending_events_.push_back(GetKeyEvent_(false, key, mods));
    }
}

GLFWViewer & GLFWViewer::GetInstance_(GLFWwindow *window) {
    assert(window);
    GLFWViewer *instance =
        reinterpret_cast<GLFWViewer *>(glfwGetWindowUserPointer(window));
    assert(instance);
    return *instance;
}

void GLFWViewer::ErrorCallback_(int error, const char *description) {
    std::cerr << "*** GLFW Error " << error << ": " << description << "\n";
}
