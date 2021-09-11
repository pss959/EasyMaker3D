#include "Application.h"

#include <typeinfo>

#include "Assert.h"
#include "ClickInfo.h"
#include "Controller.h"
#include "Handlers/LogHandler.h"
#include "Handlers/MainHandler.h"
#include "Handlers/ShortcutHandler.h"
#include "Handlers/ViewHandler.h"
#include "Managers/AnimationManager.h"
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
#include "Util/KLog.h"
#include "VR/OpenXRVR.h"
#include "Viewers/GLFWViewer.h"
#include "Widgets/DiscWidget.h"
#include "Widgets/Slider1DWidget.h"

// ----------------------------------------------------------------------------
// Application::Context_ functions.
// ----------------------------------------------------------------------------

Application::Context_::Context_() {
}

Application::Context_::~Context_() {
    // These contain raw pointers and can be cleared without regard to order.
    handlers.clear();
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
    SG::Init();

    animation_manager_.reset(new AnimationManager);

    sg_context.reset(
        new SG::Context(
            SG::TrackerPtr(new SG::Tracker()),
            ion::gfxutils::ShaderManagerPtr(new ion::gfxutils::ShaderManager),
            ion::text::FontManagerPtr(new ion::text::FontManager)));

    scene_context_.reset(new SceneContext);

    // Make sure the scene loads properly before doing anything else. Any
    // errors will result in an exception being thrown and the application
    // exiting.
    Reader reader;
    scene = reader.ReadScene(
        Util::FilePath::GetResourcePath("scenes", "workshop.mvn"),
        *sg_context->tracker);
    scene->SetUpIon(sg_context);

    // Find necessary nodes.
    UpdateSceneContext_();

    // Required GLFW interface.
    glfw_viewer_.reset(new GLFWViewer);
    if (! glfw_viewer_->Init(window_size, scene_context_->window_camera)) {
        glfw_viewer_.reset(nullptr);
        return;
    }

    // Optional VR interface. Use an OutputMuter around initialization so that
    // error messages are not spewed when OpenXR does not detect a device.
    openxrvr_.reset(new OpenXRVR);
    if (! openxrvr_->Init(scene_context_->vr_camera))
        openxrvr_.reset(nullptr);

    renderer.reset(new Renderer(sg_context->shader_manager, ! IsVREnabled()));
    renderer->Reset(*scene);

    view_handler_.reset(new ViewHandler(scene_context_->window_camera));

    log_handler_.reset(new LogHandler);
    shortcut_handler_.reset(new ShortcutHandler(app));
    main_handler_.reset(new MainHandler());
    main_handler_->SetSceneContext(scene_context_);

    // Handlers.
    handlers.push_back(log_handler_.get());  // Has to be first.
    handlers.push_back(shortcut_handler_.get());
    handlers.push_back(view_handler_.get());
    handlers.push_back(main_handler_.get());

    // Viewers.
    viewers.push_back(glfw_viewer_.get());

    // Add VR-related items if enabled.
    if (IsVREnabled()) {
        viewers.push_back(openxrvr_.get());

        handlers.push_back(l_controller_.get());
        handlers.push_back(r_controller_.get());
    }

    // Set up scroll wheel interaction.
    auto scroll = [&](Event::Device dev, float value){
        if (dev == Event::Device::kMouse)
            scene_context_->stage->ApplyScaleChange(value);
    };

    main_handler_->GetValuatorChanged().AddObserver(scroll);

    // Connect other interaction.
    scene_context_->height_slider->GetValueChanged().AddObserver(
        [&](Widget &w, const float &val){
        scene_context_->gantry->SetHeight(Lerp(val, -10.f, 100.f)); });
    main_handler_->GetClicked().AddObserver(
        std::bind(&Application::Context_::ProcessClick_, this,
                  std::placeholders::_1));
}

void Application::Context_::ReloadScene() {
    ASSERT(scene);
    // Wipe out all shaders to avoid conflicts.
    sg_context->shader_manager.Reset(new ion::gfxutils::ShaderManager);

    try {
        Reader reader;
        SG::ScenePtr new_scene = reader.ReadScene(scene->GetPath(),
                                                  *sg_context->tracker);
        new_scene->SetUpIon(sg_context);
        scene = new_scene;
        UpdateSceneContext_();
        view_handler_->ResetView();
        scene_context_->height_slider->SetValue(
            scene_context_->height_slider->GetInitialValue());
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
        // Update the frustum used for intersection testing.
        context_.scene_context_->frustum = context_.glfw_viewer_->GetFrustum();

        // Update the MainHandler.
        context_.main_handler_->ProcessUpdate(is_alternate_mode);

        // Process any animations. Do this after updating the MainHandler
        // because a click timeout may start an animation.
        const bool is_animating = context_.animation_manager_->ProcessUpdate();

        // Let the GLFWViewer know whether to poll events or wait for events.
        // If VR is active, it needs to continuously poll events to track the
        // headset and controllers properly. This means that the GLFWViewer
        // also needs to poll events (rather than wait for them) so as not to
        // block anything. The same is true if the MainHandler is in the middle
        // of handling something (not just waiting for events) or there is an
        // animation running.
        const bool have_to_poll =
            IsVREnabled() || is_animating ||
            ! context_.main_handler_->IsWaiting();
        context_.glfw_viewer_->SetPollEventsFlag(have_to_poll);

        // Handle all incoming events.
        events.clear();
        for (auto &viewer: context_.viewers)
            viewer->EmitEvents(events);
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

        if (context_.shortcut_handler_->ShouldExit())
            keep_running = false;
    }
}

void Application::Context_::UpdateSceneContext_() {
    ASSERT(scene_context_);
    scene_context_->scene = scene;
    SceneContext &sc = *scene_context_;

    // Access the Gantry and cameras.
    sc.gantry = scene->GetGantry();
    ASSERT(sc.gantry);
    for (auto &cam: sc.gantry->GetCameras()) {
        if (cam->GetTypeName() == "WindowCamera")
            sc.window_camera = Util::CastToDerived<SG::WindowCamera>(cam);
        else if (cam->GetTypeName() == "VRCamera")
            sc.vr_camera = Util::CastToDerived<SG::VRCamera>(cam);
    }
    ASSERT(sc.window_camera);
    ASSERT(sc.vr_camera);

    // Find all of the other named nodes.
    sc.height_slider = SG::FindTypedNodeInScene<Slider1DWidget>(
        *scene, "HeightSlider");
    sc.left_controller  = SG::FindNodeInScene(*scene, "LeftController");
    sc.right_controller = SG::FindNodeInScene(*scene, "RightController");
    sc.stage = SG::FindTypedNodeInScene<DiscWidget>(*scene, "Stage");
    sc.debug_text = SG::FindTypedNodeInScene<SG::TextNode>(*scene, "DebugText");
    sc.debug_sphere = SG::FindNodeInScene(*scene, "DebugSphere");

    // And shapes.
    SG::NodePtr line_node = SG::FindNodeInScene(*scene, "Debug Line");
    sc.debug_line = Util::CastToDerived<SG::Line>(line_node->GetShapes()[0]);
    ASSERT(sc.debug_line);

    // Set up the Controller instances. Disable them if not in VR.
    l_controller_.reset(new Controller(Hand::kLeft, sc.left_controller,
                                       IsVREnabled()));
    r_controller_.reset(new Controller(Hand::kRight, sc.right_controller,
                                       IsVREnabled()));
}

void Application::Context_::ProcessClick_(const ClickInfo &info) {
    KLOG('k', "Click on widget "
         << info.widget << " is_alt = " << info.is_alternate_mode
         << " is_long = " << info.is_long_press);
    if (info.widget) {
        if (info.widget == scene_context_->stage.get()) {
            // Reset the stage if alt-clicked.
            if (info.is_alternate_mode) {
                animation_manager_->StartAnimation(
                    // XXXX Add shorthand for this to Util?
                    std::bind(&Application::Context_::ResetStage_, this,
                              scene_context_->stage->GetScale(),
                              scene_context_->stage->GetRotation(),
                              std::placeholders::_1));
            }
        }
        else {
            info.widget->Click(info);
        }
    }
    else {
        // TODO: Change selection, etc...
    }
}

bool Application::Context_::ResetStage_(const Vector3f &start_scale,
                                        const Rotationf &start_rot,
                                        float time) {
    // Maximum amount to change per second.
    static const float kMaxDeltaScale = 4.f;
    static const float kMaxDeltaAngle = 90.f;

    // Compute how long the animation should last based on the amount that the
    // scale and rotation have to change.
    Vector3f axis;
    Anglef   angle;
    start_rot.GetAxisAndAngle(&axis, &angle);
    const float max_scale = start_scale[GetMaxAbsElementIndex(start_scale)];
    const float duration = std::max(angle.Degrees() / kMaxDeltaAngle,
                                    max_scale / kMaxDeltaScale);

    // Interpolate and update the stage's scale and rotation.
    const float t = std::min(1.f, time / duration);
    DiscWidget &stage = *scene_context_->stage;
    stage.SetScale(Lerp(t, start_scale, Vector3f(1, 1, 1)));
    stage.SetRotation(Rotationf::Slerp(start_rot, Rotationf::Identity(), t));

    // Keep going until finished.
    return t < 1.f;
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
