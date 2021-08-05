#include "Application.h"

#include <assert.h>

#include <typeinfo>

#include "Controller.h"
#include "GLFWViewer.h"
#include "LogHandler.h"
#include "Renderer.h"
#include "Scene.h"
#include "Util.h"
#include "VR/OpenXRVR.h"
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
    view_handler_ = nullptr;
    scene         = nullptr;
    renderer      = nullptr;
    openxrvr_     = nullptr;
    glfw_viewer_  = nullptr;
}

void Application::Context_::Init(const Vector2i &window_size) {
    // Required GLFW interface.
    glfw_viewer_.reset(new GLFWViewer());
    if (! glfw_viewer_->Init(window_size)) {
        glfw_viewer_.reset(nullptr);
        return;
    }

    // Optional VR interface. Use an OutputMuter around initialization so that
    // error messages are not spewed when OpenXR does not detect a device.
    openxrvr_.reset(new OpenXRVR());
    {
        Util::OutputMuter muter;
        if (! openxrvr_->Init(window_size))
            openxrvr_.reset(nullptr);
    }

    renderer.reset(new Renderer);
    scene.reset(new Scene);

    view_handler_.reset(new ViewHandler);
    view_handler_->SetView(&glfw_viewer_->GetView());

    log_handler_.reset(new LogHandler);

    // Fill in the lists.
    handlers.push_back(log_handler_.get());  // Has to be first.
    viewers.push_back(glfw_viewer_.get());
    emitters.push_back(glfw_viewer_.get());
    handlers.push_back(glfw_viewer_.get());
    handlers.push_back(view_handler_.get());

    if (openxrvr_) {
        viewers.push_back(openxrvr_.get());
        emitters.push_back(openxrvr_.get());
        handlers.push_back(openxrvr_.get());

        // Also set up the Controller instances.
        l_controller_.reset(new Controller(Hand::kLeft));
        r_controller_.reset(new Controller(Hand::kRight));
        l_controller_->AddModelToScene(*scene);
        r_controller_->AddModelToScene(*scene);

        handlers.push_back(l_controller_.get());
        handlers.push_back(r_controller_.get());
    }
}
