#include "Application.h"

#include <assert.h>

#include <typeinfo>

#include "GLFWViewer.h"
#include "OpenXRVR.h"
#include "Renderer.h"
#include "Scene.h"
#include "Util.h"
#include "ViewHandler.h"

using ion::math::Vector2i;

Application::Application() {
}

Application::~Application() {
}

IApplication::Context & Application::Init(const Vector2i &window_size) {
    assert(! context_.glfw_viewer_);

    context_.Init(window_size);
    return context_;
}

// ----------------------------------------------------------------------------
// Application::Context_ functions.
// ----------------------------------------------------------------------------

Application::Context_::Context_() {
}

Application::Context_::~Context_() {
    // These contain raw pointers and can be cleared without regard to order.
    handlers.clear();
    emitters.clear();
    viewers.clear();

    // Instances must be destroyed in a particular order.
    scene        = nullptr;
    renderer     = nullptr;
    openxrvr_    = nullptr;
    glfw_viewer_ = nullptr;
}

void Application::Context_::Init(const Vector2i &window_size) {
    // Required GLFW interface.
    glfw_viewer_.reset(new GLFWViewer());
    if (! glfw_viewer_->Init(window_size)) {
        glfw_viewer_.reset(nullptr);
        return;
    }

    // Optional VR interface.
    openxrvr_.reset(new OpenXRVR());
    if (! openxrvr_->Init(window_size))
        openxrvr_.reset(nullptr);

    view_handler_.reset(new ViewHandler);
    renderer.reset(new Renderer);
    scene.reset(new Scene);

    /* XXXX
    vr.reset(openxrvr_.get());
    if (openxrvr_)
        openxrvr_->InitRendering(*renderer);
    */

    // Fill in the lists.
    viewers.push_back(glfw_viewer_.get());
    emitters.push_back(glfw_viewer_.get());
    handlers.push_back(glfw_viewer_.get());
    handlers.push_back(view_handler_.get());

    if (openxrvr_) {
        viewers.push_back(openxrvr_.get());
        emitters.push_back(openxrvr_.get());
        handlers.push_back(openxrvr_.get());
    }
}
