#include "Application.h"

#include <assert.h>

#include <typeinfo>

#include "GLFWViewer.h"
#include "OpenXRVR.h"
#include "Renderer.h"
#include "Scene.h"
#include "Util.h"

using ion::math::Vector2i;

Application::Application() {
}

Application::~Application() {
    std::cerr << "XXXX Destroying Application\n";
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
    // Instances must be destroyed in a particular order.
    handlers.clear();
    emitters.clear();
    viewers.clear();
    vr           = nullptr;
    scene        = nullptr;
    renderer     = nullptr;
    openxrvr_    = nullptr;
    glfw_viewer_ = nullptr;
}

void Application::Context_::Init(const Vector2i &window_size) {
    // Required GLFW interface.
    glfw_viewer_ = std::make_shared<GLFWViewer>();
    if (! glfw_viewer_->Init(window_size)) {
        glfw_viewer_ = nullptr;
        return;
    }

    // Optional VR interface.
    openxrvr_ = std::make_shared<OpenXRVR>();
    if (! openxrvr_->Init())
        openxrvr_ = nullptr;

    renderer.reset(new Renderer);
    scene.reset(new Scene);
    vr = openxrvr_;  // XXXX

    if (vr)
        vr->InitRendering(*renderer);

    // Fill in the lists.
    if (glfw_viewer_) {
        viewers.push_back(Util::CastToBase<IViewer>(glfw_viewer_));
        emitters.push_back(Util::CastToBase<IEmitter>(glfw_viewer_));
        // XXXX handlers.push_back(glfw_viewer);
    }
    if (vr) {
        // XXXX viewers.push_back(std::shared_ptr<IViewer>(vr));
        // XXXX emitters.push_back(interfaces_.vr);
    }
}
