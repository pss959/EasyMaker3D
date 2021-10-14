#include "Application.h"

#include <typeinfo>

#include "Assert.h"
#include "ClickInfo.h"
#include "Commands/CreatePrimitiveModelCommand.h"
#include "Controller.h"
#include "Executors/CreatePrimitiveExecutor.h"
#include "Executors/TranslateExecutor.h"
#include "Feedback/LinearFeedback.h"
#include "Handlers/LogHandler.h"
#include "Handlers/MainHandler.h"
#include "Handlers/ShortcutHandler.h"
#include "Handlers/ViewHandler.h"
#include "IO/Reader.h"
#include "Items/Shelf.h"
#include "Managers/AnimationManager.h"
#include "Managers/ColorManager.h"
#include "Managers/CommandManager.h"
#include "Managers/FeedbackManager.h"
#include "Managers/IconManager.h"
#include "Managers/NameManager.h"
#include "Managers/PrecisionManager.h"
#include "Managers/ToolManager.h"
#include "Math/Animation.h"
#include "Math/Types.h"
#include "Procedural.h"
#include "RegisterTypes.h"
#include "Renderer.h"
#include "SG/Camera.h"
#include "SG/Change.h"
#include "SG/Init.h"
#include "SG/Node.h"
#include "SG/ProceduralImage.h"
#include "SG/Search.h"
#include "SG/ShaderProgram.h"
#include "SG/TextNode.h"
#include "SG/Tracker.h"
#include "Tools/Tool.h"
#include "Util/FilePath.h"
#include "Util/General.h"
#include "Util/KLog.h"
#include "VR/VRContext.h"
#include "Viewers/GLFWViewer.h"
#include "Viewers/VRViewer.h"
#include "Widgets/DiscWidget.h"
#include "Widgets/PushButtonWidget.h"
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
    vr_context_   = nullptr;
    glfw_viewer_  = nullptr;
}

void Application::Context_::Init(const Vector2i &window_size,
                                 IApplication &app) {
    // Register all known concrete types with the Parser::Registry.
    RegisterTypes();

    SG::Init();

    tracker.reset(new SG::Tracker());
    shader_manager.Reset(new ion::gfxutils::ShaderManager);
    font_manager.Reset(new ion::text::FontManager);
    ion_context_.reset(new SG::IonContext());
    ion_context_->SetTracker(tracker);
    ion_context_->SetShaderManager(shader_manager);
    ion_context_->SetFontManager(font_manager);

    scene_context_.reset(new SceneContext);

    // Make sure the scene loads properly before doing anything else. Any
    // errors will result in an exception being thrown and the application
    // exiting.
    Reader reader;
    scene = reader.ReadScene(
        Util::FilePath::GetResourcePath("scenes", "workshop.mvn"), *tracker);
    scene->SetUpIon(ion_context_);

    // Find necessary nodes.
    UpdateSceneContext_();

    // Required GLFW interface.
    glfw_viewer_.reset(new GLFWViewer);
    if (! glfw_viewer_->Init(window_size)) {
        glfw_viewer_.reset(nullptr);
        return;
    }

    // Optional VR interface.
    vr_context_.reset(new VRContext);
    if (! vr_context_->Init())
        vr_context_.reset(nullptr);

    renderer.reset(new Renderer(shader_manager, ! IsVREnabled()));
    renderer->Reset(*scene);

    view_handler_.reset(new ViewHandler());

    log_handler_.reset(new LogHandler);
    shortcut_handler_.reset(new ShortcutHandler(app));
    main_handler_.reset(new MainHandler());

    // Handlers.
    handlers.push_back(log_handler_.get());  // Has to be first.
    handlers.push_back(shortcut_handler_.get());
    handlers.push_back(view_handler_.get());
    handlers.push_back(main_handler_.get());

    // Viewers.
    viewers.push_back(glfw_viewer_.get());

    // Add VR-related items if enabled.
    if (IsVREnabled()) {
        vr_context_->InitRendering(*renderer);

        vr_viewer_.reset(new VRViewer(*vr_context_));
        viewers.push_back(vr_viewer_.get());

        handlers.push_back(l_controller_.get());
        handlers.push_back(r_controller_.get());
    }

    // Set up scroll wheel interaction.
    auto scroll = [&](Event::Device dev, float value){
        if (dev == Event::Device::kMouse)
            scene_context_->stage->ApplyScaleChange(value);
    };

    main_handler_->GetValuatorChanged().AddObserver(this, scroll);
    main_handler_->GetClicked().AddObserver(
        this, std::bind(&Application::Context_::ProcessClick_, this,
                        std::placeholders::_1));

    // Connect interaction in the scene.
    ConnectSceneInteraction_();

    // Set up managers.
    animation_manager_.reset(new AnimationManager);
    color_manager_.reset(new ColorManager);
    feedback_manager_.reset(new FeedbackManager);
    command_manager_.reset(new CommandManager);
    icon_manager_.reset(new IconManager);
    name_manager_.reset(new NameManager);
    precision_manager_.reset(new PrecisionManager);
    tool_manager_.reset(new ToolManager);
    selection_manager_.reset(new SelectionManager(scene_context_->root_model));

    // Install things...
    main_handler_->SetPrecisionManager(precision_manager_);
    main_handler_->SetSceneContext(scene_context_);

    // Set up executors.
    std::shared_ptr<Executor::Context> exec_context(new Executor::Context);
    exec_context->root_model        = scene_context_->root_model;
    exec_context->animation_manager = animation_manager_;
    exec_context->color_manager     = color_manager_;
    exec_context->name_manager      = name_manager_;
    exec_context->selection_manager = selection_manager_;

    // Detect selection changes to update the ToolManager.
    selection_manager_->GetSelectionChanged().AddObserver(
        this, std::bind(&Application::Context_::SelectionChanged_, this,
                        std::placeholders::_1, std::placeholders::_2));

    executors_.push_back(ExecutorPtr(new CreatePrimitiveExecutor));
    executors_.push_back(ExecutorPtr(new TranslateExecutor));
    for (auto &exec: executors_) {
        exec->SetContext(exec_context);
        auto func = [exec](Command &cmd,
                           Command::Op op){ exec->Execute(cmd, op); };
        command_manager_->RegisterFunction(exec->GetCommandTypeName(), func);
    }

    // Set up the icons on the shelves.
    const Point3f cam_pos = glfw_viewer_->GetFrustum().position;
    const SG::NodePtr shelf_geometry = SG::FindNodeInScene(*scene, "Shelf");
    ShelfPtr creation_shelf =
        SG::FindTypedNodeInScene<Shelf>(*scene, "CreationShelf");
    const float distance =
        ion::math::Distance(cam_pos, Point3f(creation_shelf->GetTranslation()));
    std::vector<WidgetPtr> creation_widgets;
    creation_widgets.push_back(SetUpPushButton_("CreateBoxIcon",
                                                Action::kCreateBox));
    creation_widgets.push_back(SetUpPushButton_("CreateCylinderIcon",
                                                Action::kCreateCylinder));
    creation_widgets.push_back(SetUpPushButton_("CreateSphereIcon",
                                                Action::kCreateSphere));
    creation_widgets.push_back(SetUpPushButton_("CreateTorusIcon",
                                                Action::kCreateTorus));
    creation_shelf->Init(shelf_geometry, creation_widgets, distance);
    Util::AppendVector(creation_widgets, icon_widgets_);

    // Set up Tool::Context, the Tools, and the ToolManager.
    tool_context_.reset(new Tool::Context);
    tool_context_->color_manager     = color_manager_;
    tool_context_->command_manager   = command_manager_;
    tool_context_->feedback_manager  = feedback_manager_;
    tool_context_->precision_manager = precision_manager_;
    // XXXX More...
    const SG::NodePtr tool_parent = SG::FindNodeInScene(*scene, "ToolParent");
    tool_manager_->SetParentNode(tool_parent);
    GeneralToolPtr trans_tool =
        SG::FindTypedNodeInScene<GeneralTool>(*scene, "TranslationTool");
    trans_tool->SetContext(tool_context_);
    tool_manager_->AddGeneralTool(trans_tool);
    tool_manager_->SetDefaultGeneralTool(trans_tool);

    // Set up the FeedbackManager.
    const SG::NodePtr fb_parent = SG::FindNodeInScene(*scene, "FeedbackParent");
    feedback_manager_->SetParentNode(fb_parent);
    feedback_manager_->SetSceneBoundsFunc([this](){
        return scene_context_->root_model->GetBounds(); });
    feedback_manager_->AddTemplate<LinearFeedback>(
        SG::FindTypedNodeInScene<LinearFeedback>(*scene, "LinearFeedback"));
    // XXXX More...

    // Allow new Tooltip instances to be created.
    Tooltip::SetCreationFunc([this](){
        return Util::CastToDerived<Tooltip>(
            scene_context_->tooltip->Clone(true)); });
}

void Application::Context_::SelectionChanged_(const Selection &sel,
                                              SelectionManager::Operation op) {
    switch (op) {
      case SelectionManager::Operation::kSelection:
        tool_manager_->AttachToSelection(sel);
        break;
      case SelectionManager::Operation::kReselection:
        tool_manager_->ReattachTools();
        break;
      case SelectionManager::Operation::kDeselection:
        tool_manager_->DetachTools(sel);
        break;
      case SelectionManager::Operation::kUpdate:
        // Nothing to do in this case.
        break;
    }
}

WidgetPtr Application::Context_::SetUpPushButton_(const std::string &name,
                                                  Action action) {
    PushButtonWidgetPtr but = SG::FindTypedNodeInScene<PushButtonWidget>(
         *scene_context_->scene, name);
    but->SetEnableFunction(
        std::bind(&Application::Context_::CanApplyAction, this, action));
    but->GetClicked().AddObserver(this, [this, action](const ClickInfo &info){
        ApplyAction(action);
    });
    return but;
}

void Application::Context_::CreatePrimitiveModel_(PrimitiveType type) {
    CreatePrimitiveModelCommandPtr cpc =
        Parser::Registry::CreateObject<CreatePrimitiveModelCommand>(
            "CreatePrimitiveModelCommand");
    cpc->SetType(type);
    command_manager_->AddAndDo(cpc);
    tool_manager_->UseSpecializedTool(selection_manager_->GetSelection());
}

void Application::Context_::ReloadScene() {
    ASSERT(scene);
    // Wipe out all shaders to avoid conflicts.
    shader_manager.Reset(new ion::gfxutils::ShaderManager);
    ion_context_->Reset();
    ion_context_->SetShaderManager(shader_manager);

    try {
        Reader reader;
        SG::ScenePtr new_scene = reader.ReadScene(scene->GetPath(), *tracker);
        new_scene->SetUpIon(ion_context_);
        scene = new_scene;
        UpdateSceneContext_();
        ConnectSceneInteraction_();
        view_handler_->ResetView();
        renderer->Reset(*scene);
    }
    catch (std::exception &ex) {
        std::cerr << "*** Caught exception reloading scene:\n"
                  << ex.what() << "\n";
    }
}

bool Application::Context_::CanApplyAction(Action action) {
    // XXXX Need to flesh this out...
    switch (action) {

      case Action::kUndo:
        return command_manager_->CanUndo();
      case Action::kRedo:
        return command_manager_->CanRedo();

      default:
        // Anything else is assumed to always be possible.
        return true;
    }
}

void Application::Context_::ApplyAction(Action action) {
    ASSERT(CanApplyAction(action));

    // XXXX Need to flesh this out...
    switch (action) {
      case Action::kUndo:
        command_manager_->Undo();
        break;
      case Action::kRedo:
        command_manager_->Redo();
        break;
      case Action::kQuit:
        keep_running_ = false;
        break;

      case Action::kCreateBox:
        CreatePrimitiveModel_(PrimitiveType::kBox);
        break;
      case Action::kCreateCylinder:
        CreatePrimitiveModel_(PrimitiveType::kCylinder);
        break;
      case Action::kCreateSphere:
        CreatePrimitiveModel_(PrimitiveType::kSphere);
        break;
      case Action::kCreateTorus:
        CreatePrimitiveModel_(PrimitiveType::kTorus);
        break;

      default:
        // XXXX Do something for real.
        std::cerr << "XXXX Unimplemented action "
                  << Util::EnumName(action) << "\n";
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
    sc.tooltip = SG::FindTypedNodeInScene<Tooltip>(*scene, "Tooltip");
    sc.root_model = SG::FindTypedNodeInScene<RootModel>(*scene, "ModelRoot");
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

void Application::Context_::ConnectSceneInteraction_() {
    // Inform the viewers and ViewHandler about the cameras in the scene.
    view_handler_->SetCamera(scene_context_->window_camera);
    glfw_viewer_->SetCamera(scene_context_->window_camera);
    if (IsVREnabled())
        vr_viewer_->SetCamera(scene_context_->vr_camera);

    // Hook up the height slider.
    scene_context_->height_slider->GetValueChanged().AddObserver(
        this, [&](Widget &w, const float &val){
            scene_context_->gantry->SetHeight(Lerp(val, -10.f, 100.f)); });
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
                    std::bind(&Application::Context_::ResetStage_, this,
                              scene_context_->stage->GetScale(),
                              scene_context_->stage->GetRotation(),
                              std::placeholders::_1));
            }
        }
        else if (info.widget == scene_context_->height_slider.get()) {
            // Reset the height slider if clicked or alt-clicked.
            animation_manager_->StartAnimation(
                std::bind(&Application::Context_::ResetHeightAndView_, this,
                          scene_context_->height_slider->GetValue(),
                          scene_context_->window_camera->GetOrientation(),
                          info.is_alternate_mode,
                          std::placeholders::_1));
        }
        else {
            info.widget->Click(info);
        }
    }
    // If the intersected object is part of a Tool, process the click as if
    // it were a click on the attached Model.
    else if (ToolPtr tool = info.hit.path.FindNodeUpwards<Tool>()) {
        ASSERT(tool->GetPrimaryModel());
        // Get a path ending at the Model and use that to create a SelPath.
        const SelPath path(info.hit.path.GetSubPath(*tool->GetPrimaryModel()));
        selection_manager_->ChangeModelSelection(path, info.is_alternate_mode);
    }
    // Otherwise, the click was on a noninteractive object, so deselect.
    else {
        selection_manager_->DeselectAll();
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

bool Application::Context_::ResetHeightAndView_(float start_height,
                                                const Rotationf &start_view_rot,
                                                bool reset_view, float time) {
    // Maximum amount to change per second.
    static const float kMaxDeltaHeight = .4f;
    static const float kMaxDeltaAngle  = 90.f;

    // Compute how long the animation should last based on the amount that the
    // height and view rotation have to change.
    float duration = start_height / kMaxDeltaHeight;
    if (reset_view) {
        Vector3f axis;
        Anglef   angle;
        start_view_rot.GetAxisAndAngle(&axis, &angle);
        duration = std::max(duration, angle.Degrees() / kMaxDeltaAngle);
    }

    // Interpolate and update the height slider's height and optionally the
    // window camera view direction. Use the Dampen function to ease in and
    // ease out the animation.
    const float t = std::min(1.f, Dampen(time / duration));
    Slider1DWidget &slider = *scene_context_->height_slider;
    slider.SetValue(Lerp(t, start_height, slider.GetInitialValue()));
    if (reset_view) {
        scene_context_->window_camera->SetOrientation(
            Rotationf::Slerp(start_view_rot, Rotationf::Identity(), t));
    }
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

void Application::MainLoop() {
    std::vector<Event> events;
    bool is_alternate_mode = false;  // XXXX
    while (context_.keep_running_) {
        // Update the frustum used for intersection testing.
        context_.scene_context_->frustum = context_.glfw_viewer_->GetFrustum();

        // Update everything that needs it.
        context_.main_handler_->ProcessUpdate(is_alternate_mode);
        context_.tool_context_->is_alternate_mode = is_alternate_mode;

        // Process any animations. Do this after updating the MainHandler
        // because a click timeout may start an animation.
        const bool is_animating = context_.animation_manager_->ProcessUpdate();

        // Enable or disable all icon widgets.
        // XXXX Need to Highlight current tool icons.
        for (auto &widget: context_.icon_widgets_)
            widget->SetInteractionEnabled(widget->ShouldBeEnabled());

        // Let the GLFWViewer know whether to poll events or wait for events.
        // If VR is active, it needs to continuously poll events to track the
        // headset and controllers properly. This means that the GLFWViewer
        // also needs to poll events (rather than wait for them) so as not to
        // block anything. The same is true if the MainHandler is in the middle
        // of handling something (not just waiting for events), if there is an
        // animation running, or if something is being delayed.
        const bool have_to_poll =
            IsVREnabled() || is_animating || Util::IsDelaying() ||
            ! context_.main_handler_->IsWaiting();
        context_.glfw_viewer_->SetPollEventsFlag(have_to_poll);

        // Handle all incoming events.
        events.clear();
        for (auto &viewer: context_.viewers)
            viewer->EmitEvents(events);
        for (auto &event: events) {
            // Special case for exit events.
            if (event.flags.Has(Event::Flag::kExit)) {
                context_.keep_running_ = false;
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

void Application::ReloadScene() {
    // Reset all handlers that may be holding onto state.
    for (auto &handler: context_.handlers)
        handler->Reset();

    // Wipe out any events that may be pending in viewers.
    for (auto &viewer: context_.viewers)
        viewer->FlushPendingEvents();

    context_.ReloadScene();
}

bool Application::CanApplyAction(Action action) {
    return context_.CanApplyAction(action);
}

void Application::ApplyAction(Action action) {
    context_.ApplyAction(action);
}
