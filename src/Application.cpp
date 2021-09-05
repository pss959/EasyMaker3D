#include "Application.h"

#include <typeinfo>

#include "Assert.h"
#include "Controller.h"
#include "GLFWViewer.h"
#include "Handlers/LogHandler.h"
#include "Handlers/MainHandler.h"
#include "Handlers/ShortcutHandler.h"
#include "Handlers/ViewHandler.h"
#include "Math/Types.h"
#include "Procedural.h"
#include "Reader.h"
#include "Renderer.h"
#include "SG/Camera.h"
#include "SG/Init.h"
#include "SG/Node.h"
#include "SG/ProceduralImage.h"
#include "SG/Search.h"
#include "SG/ShaderProgram.h"
#include "SG/TextNode.h"
#include "SG/Tracker.h"
#include "Util/FilePath.h"
#include "Util/General.h"
#include "VR/OpenXRVR.h"

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

void Application::Context_::Init(const Vector2i &window_size,
                                 IApplication &app) {
    shader_manager.Reset(new ion::gfxutils::ShaderManager);
    font_manager.Reset(new ion::text::FontManager);

    SG::Init();

    tracker_.reset(new SG::Tracker);
    scene_context_.reset(new SceneContext);

    // Make sure the scene loads properly before doing anything else. Any
    // errors will result in an exception being thrown and the application
    // exiting.
    Reader reader(*tracker_, shader_manager, font_manager);
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

    renderer.reset(new Renderer(shader_manager, ! IsVREnabled()));
    renderer->Reset(*scene);

    view_handler_.reset(new ViewHandler(glfw_viewer_->GetView(),
                                        *scene_context_));

    main_handler_.reset(new MainHandler());
    log_handler_.reset(new LogHandler);
    shortcut_handler_.reset(new ShortcutHandler(app));

    // Handlers.
    handlers.push_back(log_handler_.get());  // Has to be first.
    handlers.push_back(shortcut_handler_.get());
    handlers.push_back(glfw_viewer_.get());
    handlers.push_back(view_handler_.get());
    handlers.push_back(main_handler_.get());

    // Viewers.
    viewers.push_back(glfw_viewer_.get());

    // Emitters.
    emitters.push_back(glfw_viewer_.get());

    // Add VR-related items if enabled.
    if (IsVREnabled()) {
        viewers.push_back(openxrvr_.get());
        emitters.push_back(openxrvr_.get());
        handlers.push_back(openxrvr_.get());

        handlers.push_back(l_controller_.get());
        handlers.push_back(r_controller_.get());
    }

    // Find necessary nodes.
    UpdateSceneContext_();

    // Add the scene's root node to all Views.
    UpdateViews_();

    // If VR is active, it needs to continuously poll events to track the
    // headset and controllers properly. This means that the GLFWViewer also
    // needs to poll events (rather than wait for them) so as not to block
    // anything.
    if (IsVREnabled())
        glfw_viewer_->SetPollEventsFlag(true);
}

void Application::Context_::ReloadScene() {
    ASSERT(scene);
    // Wipe out all shaders to avoid conflicts.
    shader_manager.Reset(new ion::gfxutils::ShaderManager);
    Reader reader(*tracker_, shader_manager, font_manager);

    try {
        SG::ScenePtr new_scene = reader.ReadScene(scene->GetPath());
        scene = new_scene;
        UpdateSceneContext_();
        UpdateViews_();
        view_handler_->ResetView();
        renderer->Reset(*scene);
    }
    catch (std::exception &ex) {
        std::cerr << "*** Caught exception reloading scene:\n"
                  << ex.what() << "\n";
    }
}

void Application::MainLoop() {
    std::vector<Event> events;
    bool keep_running = true;
    bool is_alternate_mode = false;  // XXXX
    while (keep_running) {
        /* XXXX Show the current frame.
        context_.scene_context_->debug_text->SetText(
            Util::ToString(context_.renderer->GetFrameCount()));
        */

        context_.main_handler_->ProcessUpdate(is_alternate_mode);

        // Handle all incoming events.
        events.clear();
        for (auto &emitter: context_.emitters)
            emitter->EmitEvents(events);
        for (auto &event: events) {
            // Special case for exit events.
            if (event.flags.Has(Event::Flag::kExit)) {
                keep_running = false;
                break;
            }
            for (auto &handler: context_.handlers)
                if (handler->HandleEvent(event))
                    break;
        }

        // Render to all viewers.
        for (auto &viewer: context_.viewers)
            viewer->Render(*context_.scene, *context_.renderer);
    }
}

void Application::Context_::UpdateSceneContext_() {
    ASSERT(scene_context_);
    scene_context_->scene = scene;

    scene_context_->debug_text =
        SG::FindTypedNodeInScene<SG::TextNode>(*scene, "DebugText");

    // XXXX Add this to Search?
    SG::NodePtr line_node = SG::FindNodeInScene(*scene, "Debug Line");
    scene_context_->debug_line = Util::CastToDerived<SG::Line>(
        line_node->GetShapes()[0]);
    ASSERT(scene_context_->debug_line);

    scene_context_->debug_sphere = SG::FindNodeInScene(*scene, "DebugSphere");

    scene_context_->left_controller =
        SG::FindNodeInScene(*scene, "LeftController");
    scene_context_->right_controller =
        SG::FindNodeInScene(*scene, "RightController");

    // Also set up the Controller instances. Disable them if not in VR.
    l_controller_.reset(new Controller(
                            Hand::kLeft, scene_context_->left_controller,
                            IsVREnabled()));
    r_controller_.reset(new Controller(
                            Hand::kRight, scene_context_->right_controller,
                            IsVREnabled()));

    // Inform the MainHandler.
    main_handler_->SetSceneContext(scene_context_);
}

void Application::Context_::UpdateViews_() {
    ASSERT(scene);
    if (scene->GetCamera()) {
        View &view = glfw_viewer_->GetView();
        view.SetFrustum(scene->GetCamera()->BuildFrustum(
                            view.GetAspectRatio()));
        if (IsVREnabled())
            openxrvr_->SetBaseViewPosition(scene->GetCamera()->GetPosition());
    }
    // Set the frustum in the SceneContext for ray intersections.
    scene_context_->frustum = glfw_viewer_->GetView().GetFrustum();
}

// ----------------------------------------------------------------------------
// Application functions.
// ----------------------------------------------------------------------------

Application::Application() {
}

Application::~Application() {
}

void Application::Init(const Vector2i &window_size) {
    // TODO: Compute this dynamically.
    const float kStageRadius = 32.f;
    // Register procedural functions before reading the scene.
    SG::ProceduralImage::AddFunction(
        "GenerateGridImage", std::bind(GenerateGridImage, kStageRadius));

    ASSERT(! context_.glfw_viewer_);
    context_.Init(window_size, *this);
}

IApplication::Context & Application::GetContext() {
    return context_;
}

void Application::ReloadScene() {
    context_.ReloadScene();
}

