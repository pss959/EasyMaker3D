#include "Application.h"

#include <assert.h>

#include <typeinfo>

#include "Controller.h"
#include "Frustum.h"
#include "GLFWViewer.h"
#include "LogHandler.h"
#include "Renderer.h"
#include "SG/Camera.h"
#include "SG/Node.h"
#include "SG/Reader.h"
#include "SG/Tracker.h"
#include "ShortcutHandler.h"
#include "Util/FilePath.h"
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
    shader_manager.Reset(new ion::gfxutils::ShaderManager);

    tracker_.reset(new SG::Tracker);

    // Make sure the scene loads properly before doing anything else. Any
    // errors will result in an exception being thrown and the application
    // exiting.
    SG::Reader reader(*tracker_, *shader_manager);
    scene = reader.ReadScene(
        Util::FilePath::GetResourcePath("scenes", "workshop.mvn"));

    // Required GLFW interface.
    glfw_viewer_.reset(new GLFWViewer);
    if (! glfw_viewer_->Init(window_size)) {
        glfw_viewer_.reset(nullptr);
        return;
    }
    // Initialize the GLFWViewer from the Scene's camera, if any.
    if (scene->GetCamera()) {
        View &view = glfw_viewer_->GetView();
        view.SetFrustum(scene->GetCamera()->BuildFrustum(
                            view.GetAspectRatio()));
    }

    // Optional VR interface. Use an OutputMuter around initialization so that
    // error messages are not spewed when OpenXR does not detect a device.
    openxrvr_.reset(new OpenXRVR);
    if (! openxrvr_->Init(window_size))
        openxrvr_.reset(nullptr);

    renderer.reset(new Renderer(shader_manager));

    view_handler_.reset(new ViewHandler(glfw_viewer_->GetView()));

    log_handler_.reset(new LogHandler);
    shortcut_handler_.reset(new ShortcutHandler(*this));

    // Handlers.
    handlers.push_back(log_handler_.get());  // Has to be first.
    handlers.push_back(shortcut_handler_.get());
    handlers.push_back(glfw_viewer_.get());
    handlers.push_back(view_handler_.get());

    // Viewers.
    viewers.push_back(glfw_viewer_.get());

    // Emitters.
    emitters.push_back(glfw_viewer_.get());

    // Add VR-related items if enabled.
    if (openxrvr_) {
        viewers.push_back(openxrvr_.get());
        emitters.push_back(openxrvr_.get());
        handlers.push_back(openxrvr_.get());

        // Also set up the Controller instances.
        /* XXXX Need to find nodes in scene.
        l_controller_.reset(new Controller(Hand::kLeft,  scene));
        r_controller_.reset(new Controller(Hand::kRight, scene));
        */

        handlers.push_back(l_controller_.get());
        handlers.push_back(r_controller_.get());
    }

    // Add the scene's root node to all Views.
    const auto &root = scene->GetRootNode()->GetIonNode();
    for (auto &viewer: viewers)
        viewer->GetView().AddNode(root);

    // If VR is active, it needs to continuously poll events to track the
    // headset and controllers properly. This means that the GLFWViewer also
    // needs to poll events (rather than wait for them) so as not to block
    // anything.
    if (openxrvr_)
        glfw_viewer_->SetPollEventsFlag(true);
}
