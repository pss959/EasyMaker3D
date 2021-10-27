#include "Application.h"

#include <typeinfo>

#include "Assert.h"
#include "ClickInfo.h"
#include "Controller.h"
#include "Enums/PrimitiveType.h"
#include "Executors/CreatePrimitiveExecutor.h"
#include "Executors/TranslateExecutor.h"
#include "Feedback/LinearFeedback.h"
#include "Handlers/LogHandler.h"
#include "Handlers/MainHandler.h"
#include "Handlers/ShortcutHandler.h"
#include "Handlers/ViewHandler.h"
#include "IO/Reader.h"
#include "Items/Board.h"
#include "Items/Icon.h"
#include "Items/Shelf.h"
#include "Managers/ActionManager.h"
#include "Managers/AnimationManager.h"
#include "Managers/ColorManager.h"
#include "Managers/CommandManager.h"
#include "Managers/FeedbackManager.h"
#include "Managers/NameManager.h"
#include "Managers/PrecisionManager.h"
#include "Managers/TargetManager.h"
#include "Managers/ToolManager.h"
#include "Math/Animation.h"
#include "Math/Types.h"
#include "Procedural.h"
#include "RegisterTypes.h"
#include "Renderer.h"
#include "SG/Camera.h"
#include "SG/Change.h"
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

#include "Panes/TextPane.h"  // XXXX

// ----------------------------------------------------------------------------
// Application::Loader_ class.
// ----------------------------------------------------------------------------

/// Application::Loader_ does most of the work of loading a scene. It manages
/// all context necessary to read the scene and set it up.
class Application::Loader_ {
  public:
    Loader_();

    /// Reads a scene from the given path. Updates the SceneContext with the
    /// necessary items. Returns a null SG::ScenePtr if anything fails.
    SG::ScenePtr LoadScene(const Util::FilePath &path,
                           SceneContext &scene_context);

    const ion::gfxutils::ShaderManagerPtr & GetShaderManager() {
        return shader_manager_;
    }

  private:
    SG::TrackerPtr                  tracker_;
    ion::gfxutils::ShaderManagerPtr shader_manager_;
    ion::text::FontManagerPtr       font_manager_;
    SG::IonContextPtr               ion_context_;

    /// Fills in the SceneContext from the given scene. Asserts if anything is
    /// missing or bad.
    void FillSceneContext_(const SG::ScenePtr &scene, SceneContext &sc);
};

Application::Loader_::Loader_() :
    tracker_(new SG::Tracker),
    shader_manager_(new ion::gfxutils::ShaderManager),
    font_manager_(new ion::text::FontManager),
    ion_context_(new SG::IonContext) {

    ion_context_->SetTracker(tracker_);
    ion_context_->SetShaderManager(shader_manager_);
    ion_context_->SetFontManager(font_manager_);
}

SG::ScenePtr Application::Loader_::LoadScene(const Util::FilePath &path,
                                             SceneContext &scene_context) {
    // Wipe out all previous shaders to avoid conflicts.
    shader_manager_.Reset(new ion::gfxutils::ShaderManager);
    ion_context_->Reset();
    ion_context_->SetShaderManager(shader_manager_);

    SG::ScenePtr scene;
    try {
        Reader reader;
        scene = reader.ReadScene(path, *tracker_);
        scene->SetUpIon(ion_context_);

        FillSceneContext_(scene, scene_context);

    }
    catch (std::exception &ex) {
        std::cerr << "*** Caught exception loading scene:\n"
                  << ex.what() << "\n";
        scene.reset();
    }
    return scene;
}

void Application::Loader_::FillSceneContext_(const SG::ScenePtr &scene,
                                             SceneContext &sc) {
    sc.scene = scene;

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
    sc.room = SG::FindNodeInScene(*scene, "Room");
    sc.stage = SG::FindTypedNodeInScene<DiscWidget>(*scene, "Stage");
    sc.tooltip = SG::FindTypedNodeInScene<Tooltip>(*scene, "Tooltip");
    sc.root_model = SG::FindTypedNodeInScene<RootModel>(*scene, "ModelRoot");
    sc.debug_text = SG::FindTypedNodeInScene<SG::TextNode>(*scene, "DebugText");
    sc.debug_sphere = SG::FindNodeInScene(*scene, "DebugSphere");

    // And shapes.
    SG::NodePtr line_node = SG::FindNodeInScene(*scene, "Debug Line");
    sc.debug_line = Util::CastToDerived<SG::Line>(line_node->GetShapes()[0]);
    ASSERT(sc.debug_line);
}

// ----------------------------------------------------------------------------
// Application::Impl_ class. This does most of the work for the Application
// class.
// ----------------------------------------------------------------------------

class  Application::Impl_ {
  public:
    Impl_();
    ~Impl_();

    bool Init(const Vector2i &window_size);

    /// Returns true if VR is enabled (after Init() is called).
    bool IsVREnabled() const { return vr_context_.get(); }

    LogHandler & GetLogHandler() const { return *log_handler_; }

    /// Enters the main loop for the application.
    void MainLoop();

    /// Reloads the scene from its path, updating everything necessary.
    void ReloadScene();

  private:
    std::unique_ptr<Loader_>  loader_;  ///< For loading and reloading scenes.

    /// \name Managers.
    ///@{
    ActionManagerPtr    action_manager_;
    AnimationManagerPtr animation_manager_;
    ColorManagerPtr     color_manager_;
    CommandManagerPtr   command_manager_;
    FeedbackManagerPtr  feedback_manager_;
    NameManagerPtr      name_manager_;
    PrecisionManagerPtr precision_manager_;
    SelectionManagerPtr selection_manager_;
    TargetManagerPtr    target_manager_;
    ToolManagerPtr      tool_manager_;
    ///@}

    /// \name Various Contexts.
    ///@{
    SceneContextPtr  scene_context_;
    Tool::ContextPtr tool_context_;
    VRContextPtr     vr_context_;
    ///@}

    /// \name Individual Handlers.
    ///@{
    LogHandlerPtr      log_handler_;
    MainHandlerPtr     main_handler_;
    ShortcutHandlerPtr shortcut_handler_;
    ViewHandlerPtr     view_handler_;
    ///@}

    /// \name Individual Viewers.
    ///@{
    GLFWViewerPtr    glfw_viewer_;
    VRViewerPtr      vr_viewer_;
    ///@}

    ControllerPtr    l_controller_;  ///< Left hand controller.
    ControllerPtr    r_controller_;  ///< Right hand controller.

    /// The renderer.
    RendererPtr      renderer_;

    /// All Handlers, in order.
    std::vector<HandlerPtr>   handlers_;

    /// All Viewers.
    std::vector<ViewerPtr>    viewers_;

    /// Registered Executor instances.
    std::vector<ExecutorPtr>  executors_;

    /// Executor::Context used to set up all Executor instances.
    Executor::ContextPtr      exec_context_;

    /// ActionManager::Context used to set up the ActionManager.
    ActionManager::ContextPtr action_context_;

    /// All 3D icons that need to be updated every frame.
    std::vector<IconPtr>      icons_;

    /// Set to true when anything in the scene changes.
    bool                      scene_changed_ = true;

    /// Set to false when the main loop should exit.
    bool                      keep_running_ = true;

    /// \name One-time Initialization
    /// Each of these functions sets up items that are needed by the
    /// application. They are called one time only.
    ///@{

    /// Sets up SG and registers all types.
    void InitTypes_();

    /// Initializes the GLFWViewer and optionally the VRViewer, adding them to
    /// the viewers_ vector. Returns false if the GLFWViewer could not be set
    /// up.
    bool InitViewers_(const Vector2i &window_size);

    /// Initializes all Handlers, adding them to the handlers_ vector.
    void InitHandlers_();

    /// Initializes all Managers.
    void InitManagers_();

    /// Initializes all Executors, adding them to the executors_ vector.
    void InitExecutors_();

    /// Sets up a Tool::Context, the Tools, and installs them in the
    /// ToolManager.
    void InitTools_();

    /// Sets up tooltips, allowing new Tooltip instances to be created.
    void InitTooltips_();

    /// Initializes interaction that is not dependent on items in the Scene.
    void InitInteraction_();

    ///@}

    /// \name Scene-dependent Initialization

    /// These function initialize some aspect of interaction for the
    /// application. They are called each time the scene is loaded or reloaded.
    ///@{

    /// Wires up all interaction that depends on items in the scene.
    void ConnectSceneInteraction_();

    /// Adds templates for all Tools to the ToolManager.
    void AddTools_();

    /// Adds templates for all Feedback types to the FeedbackManager.
    void AddFeedback_();

    /// Adds the 3D icons on the shelves.
    void AddIcons_();

    /// Adds the Board instances for 2D-ish UI.
    void AddBoards_();

    ///@}

    void SelectionChanged_(const Selection &sel,
                           SelectionManager::Operation op);

    // XXXX
    WidgetPtr SetUpPushButton_(const std::string &name, Action action);
    void CreatePrimitiveModel_(PrimitiveType type);

    /// Processes a click on something in the scene.
    void ProcessClick_(const ClickInfo &info);

    /// Animation callback function to reset the stage.
    bool ResetStage_(const Vector3f &start_scale,
                     const Rotationf &start_rot, float time);

    /// Animation callback function to reset the height and optionally the
    /// view direction.
    bool ResetHeightAndView_(float start_height,
                             const Rotationf &start_view_rot,
                             bool reset_view, float time);
};

// ----------------------------------------------------------------------------
// Application::Context_ functions.
// ----------------------------------------------------------------------------

Application::Impl_::Impl_() : loader_(new Loader_) {
}

Application::Impl_::~Impl_() {
    handlers_.clear();
    viewers_.clear();

    // Instances must be destroyed in a particular order.
    view_handler_.reset();
    scene_context_.reset();
    renderer_.reset();
    vr_context_.reset();
    glfw_viewer_.reset();
}

bool Application::Impl_::Init(const Vector2i &window_size) {
    // Note that order here is extremely important!

    InitTypes_();

    // Make sure the scene loads properly and has all of the necessary items
    // (in the SceneContext) before doing anything else.
    scene_context_.reset(new SceneContext);
    const Util::FilePath scene_path =
        Util::FilePath::GetResourcePath("scenes", "workshop.mvn");
    SG::ScenePtr scene = loader_->LoadScene(scene_path, *scene_context_);
    if (! scene)
        return false;

    // Set up the viewers. This also sets up the VRContext if VR is enabled so
    // that IsVREnabled() returns a valid value.
    if (! InitViewers_(window_size))
        return false;

    // Set up the renderer.
    renderer_.reset(new Renderer(loader_->GetShaderManager(), ! IsVREnabled()));
    renderer_->Reset(*scene);
    if (IsVREnabled())
        vr_context_->InitRendering(*renderer_);

    // Set up the Controller instances. Disable them if not in VR.
    l_controller_.reset(new Controller(Hand::kLeft));
    r_controller_.reset(new Controller(Hand::kRight));

    // This needs to exist for the ActionManager.
    tool_context_.reset(new Tool::Context);

    InitHandlers_();
    InitManagers_();
    InitExecutors_();
    InitTools_();
    InitTooltips_();
    InitInteraction_();

    // Install things...
    main_handler_->SetPrecisionManager(precision_manager_);
    shortcut_handler_->SetActionManager(action_manager_);

    ConnectSceneInteraction_();

    return true;
}

void Application::Impl_::MainLoop() {
    std::vector<Event> events;
    bool is_alternate_mode = false;  // XXXX
    while (keep_running_) {
        // Update the frustum used for intersection testing.
        scene_context_->frustum = glfw_viewer_->GetFrustum();

        // Update everything that needs it.
        main_handler_->ProcessUpdate(is_alternate_mode);
        tool_context_->is_alternate_mode = is_alternate_mode;

        // Process any animations. Do this after updating the MainHandler
        // because a click timeout may start an animation.
        const bool is_animating = animation_manager_->ProcessUpdate();

        // Enable or disable all icon widgets and update tooltips.
        // XXXX Need to Highlight current tool icons.
        for (auto &icon: icons_) {
            auto &widget = icon->GetWidget();
            ASSERT(widget);
            const bool enabled = widget->ShouldBeEnabled();
            widget->SetInteractionEnabled(enabled);
            if (enabled)
                widget->SetTooltipText(
                    action_manager_->GetActionTooltip(icon->GetAction()));
        }

        // Let the GLFWViewer know whether to poll events or wait for events.
        // If VR is active, it needs to continuously poll events to track the
        // headset and controllers properly. This means that the GLFWViewer
        // also needs to poll events (rather than wait for them) so as not to
        // block anything. The same is true if the MainHandler is in the middle
        // of handling something (not just waiting for events), if there is an
        // animation running, if something is being delayed, or if something
        // changed in the scene.
        const bool have_to_poll =
            IsVREnabled() || is_animating || Util::IsDelaying() ||
            scene_changed_ || ! main_handler_->IsWaiting();
        glfw_viewer_->SetPollEventsFlag(have_to_poll);

        // Handle all incoming events.
        events.clear();
        for (auto &viewer: viewers_)
            viewer->EmitEvents(events);
        for (auto &event: events) {
            // Special case for exit events.
            if (event.flags.Has(Event::Flag::kExit)) {
                keep_running_ = false;
                break;
            }
            for (auto &handler: handlers_)
                if (handler->HandleEvent(event))
                    break;
        }

        // Render to all viewers.
        for (auto &viewer: viewers_)
            viewer->Render(*scene_context_->scene, *renderer_);

        scene_changed_ = false;

        // Check for action resulting in quitting.
        if (action_manager_->ShouldQuit())
            keep_running_ = false;
    }
}

void Application::Impl_::ReloadScene() {
    ASSERT(scene_context_);
    ASSERT(scene_context_->scene);

    color_manager_->Reset();

    // Reset all handlers that may be holding onto state.
    for (auto &handler: handlers_)
        handler->Reset();

    // Wipe out any events that may be pending in viewers.
    for (auto &viewer: viewers_)
        viewer->FlushPendingEvents();

    // Wipe out all shaders to avoid conflicts.
    try {
        SG::ScenePtr scene =
            loader_->LoadScene(scene_context_->scene->GetPath(),
                               *scene_context_);
        ConnectSceneInteraction_();
        view_handler_->ResetView();
        renderer_->Reset(*scene);
    }
    catch (std::exception &ex) {
        std::cerr << "*** Caught exception reloading scene:\n"
                  << ex.what() << "\n";
    }
}

void Application::Impl_::InitTypes_() {
    // TODO: Compute this dynamically?
    const float kStageRadius = 32.f;
    // Register procedural functions before reading the scene.
    SG::ProceduralImage::AddFunction(
        "GenerateGridImage", std::bind(GenerateGridImage, kStageRadius));

    // Register all known concrete types with the Parser::Registry.
    RegisterTypes();
}

bool Application::Impl_::InitViewers_(const Vector2i &window_size) {
    // Required GLFW viewer.
    glfw_viewer_.reset(new GLFWViewer);
    if (! glfw_viewer_->Init(window_size)) {
        glfw_viewer_.reset();
        return false;
    }
    viewers_.push_back(glfw_viewer_);

    // Optional VR viewer.
    vr_context_.reset(new VRContext);
    if (vr_context_->Init()) {
        vr_viewer_.reset(new VRViewer(*vr_context_));
        viewers_.push_back(vr_viewer_);
    }
    else {
        vr_context_.reset();
    }
    return true;
}

void Application::Impl_::InitHandlers_() {
    log_handler_.reset(new LogHandler);
    shortcut_handler_.reset(new ShortcutHandler);
    view_handler_.reset(new ViewHandler());
    main_handler_.reset(new MainHandler);
    handlers_.push_back(log_handler_);  // Has to be first.
    handlers_.push_back(shortcut_handler_);
    handlers_.push_back(view_handler_);
    handlers_.push_back(main_handler_);

    if (IsVREnabled()) {
        ASSERT(l_controller_);
        handlers_.push_back(l_controller_);
        handlers_.push_back(r_controller_);
    }
}

void Application::Impl_::InitManagers_() {
    ASSERT(main_handler_);
    ASSERT(tool_context_);

    animation_manager_.reset(new AnimationManager);
    color_manager_.reset(new ColorManager);
    feedback_manager_.reset(new FeedbackManager);
    command_manager_.reset(new CommandManager);
    name_manager_.reset(new NameManager);
    precision_manager_.reset(new PrecisionManager);
    selection_manager_.reset(new SelectionManager());
    target_manager_.reset(new TargetManager(command_manager_));
    tool_manager_.reset(new ToolManager(*target_manager_));

    // The ActionManager requires its own context.
    action_context_.reset(new ActionManager::Context);
    action_context_->tool_context      = tool_context_;
    action_context_->command_manager   = command_manager_;
    action_context_->selection_manager = selection_manager_;
    action_context_->target_manager    = target_manager_;
    action_context_->tool_manager      = tool_manager_;
    action_context_->main_handler      = main_handler_;
    action_manager_.reset(new ActionManager(action_context_));
    action_manager_->SetReloadFunc(std::bind(&Impl_::ReloadScene, this));
}

void Application::Impl_::InitExecutors_() {
    ASSERT(command_manager_);

    exec_context_.reset(new Executor::Context);
    exec_context_->animation_manager = animation_manager_;
    exec_context_->color_manager     = color_manager_;
    exec_context_->name_manager      = name_manager_;
    exec_context_->selection_manager = selection_manager_;

    executors_.push_back(ExecutorPtr(new CreatePrimitiveExecutor));
    executors_.push_back(ExecutorPtr(new TranslateExecutor));
    for (auto &exec: executors_) {
        exec->SetContext(exec_context_);
        auto func = [exec](Command &cmd,
                           Command::Op op){ exec->Execute(cmd, op); };
        command_manager_->RegisterFunction(exec->GetCommandTypeName(), func);
    }
}

void Application::Impl_::InitInteraction_() {
    ASSERT(main_handler_);

    // Set up scroll wheel interaction.
    auto scroll = [&](Event::Device dev, float value){
        if (dev == Event::Device::kMouse)
            scene_context_->stage->ApplyScaleChange(value);
    };

    main_handler_->GetValuatorChanged().AddObserver(this, scroll);
    main_handler_->GetClicked().AddObserver(
        this, std::bind(&Impl_::ProcessClick_, this, std::placeholders::_1));

    // Detect selection changes to update the ToolManager.
    selection_manager_->GetSelectionChanged().AddObserver(
        this, std::bind(&Impl_::SelectionChanged_, this,
                        std::placeholders::_1, std::placeholders::_2));
}

void Application::Impl_::InitTools_() {
    ASSERT(tool_manager_);
    ASSERT(tool_context_);

    tool_context_->color_manager     = color_manager_;
    tool_context_->command_manager   = command_manager_;
    tool_context_->feedback_manager  = feedback_manager_;
    tool_context_->precision_manager = precision_manager_;
}

void Application::Impl_::InitTooltips_() {
    Tooltip::SetCreationFunc([this](){
        return scene_context_->tooltip->CloneTyped<Tooltip>(true); });
}

void Application::Impl_::ConnectSceneInteraction_() {
    ASSERT(scene_context_);
    ASSERT(scene_context_->scene);

    // Tell the ActionManager::Context about the new Scene.
    action_context_->scene = scene_context_->scene;

    // Tell the SelectionManager and Executor::Context about the new RootModel.
    selection_manager_->SetRootModel(scene_context_->root_model);
    exec_context_->root_model = scene_context_->root_model;

    main_handler_->SetSceneContext(scene_context_);

    // Inform the viewers and ViewHandler about the cameras in the scene.
    view_handler_->SetCamera(scene_context_->window_camera);
    glfw_viewer_->SetCamera(scene_context_->window_camera);
    if (IsVREnabled())
        vr_viewer_->SetCamera(scene_context_->vr_camera);

    // Set Nodes in the Controllers.
    l_controller_->SetNode(scene_context_->left_controller,  IsVREnabled());
    r_controller_->SetNode(scene_context_->right_controller, IsVREnabled());

    // Hook up the height slider.
    scene_context_->height_slider->GetValueChanged().AddObserver(
        this, [&](Widget &w, const float &val){
            scene_context_->gantry->SetHeight(Lerp(val, -10.f, 100.f)); });

    // Detect changes in the scene.
    scene_context_->scene->GetRootNode()->GetChanged().AddObserver(
        this, [this](SG::Change change){ scene_changed_ = true; });

    // Add all Tools from the templates in the scene.
    AddTools_();

    // Add all Feedback instances to the manager.
    AddFeedback_();

    // Set up 3D icons on the shelves. Note that this requires the camera to be
    // in the correct position (above).
    AddIcons_();

    // Set up the Boards.
    AddBoards_();
}

void Application::Impl_::AddTools_() {
    ASSERT(tool_manager_);
    ASSERT(scene_context_);
    ASSERT(scene_context_->scene);

    SG::Scene &scene = *scene_context_->scene;

    // XXXX More...
    const SG::NodePtr tool_parent = SG::FindNodeInScene(scene, "ToolParent");
    tool_manager_->ClearTools();
    tool_manager_->SetParentNode(tool_parent);
    GeneralToolPtr trans_tool =
        SG::FindTypedNodeInScene<GeneralTool>(scene, "TranslationTool");
    trans_tool->SetContext(tool_context_);
    tool_manager_->AddGeneralTool(trans_tool);
    tool_manager_->SetDefaultGeneralTool(trans_tool);
}

void Application::Impl_::AddFeedback_() {
    ASSERT(scene_context_);
    ASSERT(scene_context_->scene);
    ASSERT(feedback_manager_);

    SG::Scene &scene = *scene_context_->scene;

    const SG::NodePtr fb_parent = SG::FindNodeInScene(scene, "FeedbackParent");
    feedback_manager_->ClearTemplates();
    feedback_manager_->SetParentNode(fb_parent);
    feedback_manager_->SetSceneBoundsFunc([this](){
        return scene_context_->root_model->GetBounds(); });
    feedback_manager_->AddTemplate<LinearFeedback>(
        SG::FindTypedNodeInScene<LinearFeedback>(scene, "LinearFeedback"));
    // XXXX More...
}

void Application::Impl_::AddIcons_() {
    ASSERT(glfw_viewer_);
    ASSERT(action_manager_);
    ASSERT(scene_context_);
    ASSERT(scene_context_->scene);

    icons_.clear();

    // Set up the icons on the shelves.
    SG::Scene &scene = *scene_context_->scene;
    const Point3f cam_pos = glfw_viewer_->GetFrustum().position;
    const SG::NodePtr shelf_geom = SG::FindNodeInScene(scene, "ShelfGeometry");
    const SG::NodePtr icon_root  = SG::FindNodeInScene(scene, "Icons");
    const SG::NodePtr shelves    = SG::FindNodeInScene(scene, "Shelves");
    for (const auto &child: shelves->GetChildren()) {
        const ShelfPtr shelf = Util::CastToDerived<Shelf>(child);
        ASSERT(shelf);
        Util::AppendVector(shelf->Init(shelf_geom, icon_root, cam_pos,
                                       *action_manager_), icons_);
    }
}

void Application::Impl_::AddBoards_() {
    ASSERT(scene_context_);
    ASSERT(scene_context_->scene);
    ASSERT(scene_context_->room);

    // Access the Board template.
    SG::Scene &scene = *scene_context_->scene;
    const BoardPtr board = SG::FindTypedNodeInScene<Board>(scene, "Board");

    // XXXX
    const BoardPtr floating_board =
        board->CloneTyped<Board>(true, "FloatingBoard");
    floating_board->SetSize(Vector2f(22, 16));
    floating_board->SetTranslation(Vector3f(0, 14, 0));
    floating_board->Show(true);

    // XXXX TESTING!
    TextPanePtr text = Parser::Registry::CreateObject<TextPane>();
    text->SetText("Test string");
    text->resize_width_ = true;
    text->resize_height_ = true;
    floating_board->SetPane(text);

    scene_context_->room->AddChild(floating_board);
}

void Application::Impl_::SelectionChanged_(const Selection &sel,
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

WidgetPtr Application::Impl_::SetUpPushButton_(const std::string &name,
                                               Action action) {
    PushButtonWidgetPtr but = SG::FindTypedNodeInScene<PushButtonWidget>(
         *scene_context_->scene, name);
    but->SetEnableFunction(
        std::bind(&ActionManager::CanApplyAction, action_manager_.get(),
                  action));
    but->GetClicked().AddObserver(this, [this, action](const ClickInfo &info){
        action_manager_->ApplyAction(action);
    });
    return but;
}

void Application::Impl_::ProcessClick_(const ClickInfo &info) {
    KLOG('k', "Click on widget "
         << info.widget << " is_alt = " << info.is_alternate_mode
         << " is_long = " << info.is_long_press);
    if (info.widget) {
        if (info.widget == scene_context_->stage.get()) {
            // Reset the stage if alt-clicked.
            if (info.is_alternate_mode) {
                animation_manager_->StartAnimation(
                    std::bind(&Impl_::ResetStage_, this,
                              scene_context_->stage->GetScale(),
                              scene_context_->stage->GetRotation(),
                              std::placeholders::_1));
            }
        }
        else if (info.widget == scene_context_->height_slider.get()) {
            // Reset the height slider if clicked or alt-clicked.
            animation_manager_->StartAnimation(
                std::bind(&Impl_::ResetHeightAndView_, this,
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

bool Application::Impl_::ResetStage_(const Vector3f &start_scale,
                                     const Rotationf &start_rot, float time) {
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

bool Application::Impl_::ResetHeightAndView_(float start_height,
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

Application::Application() : impl_(new Impl_) {
}

Application::~Application() {
}

bool Application::Init(const Vector2i &window_size) {
    return impl_->Init(window_size);
}

void Application::MainLoop() {
    impl_->MainLoop();
}

void Application::ReloadScene() {
    impl_->ReloadScene();
}

LogHandler & Application::GetLogHandler() const {
    return impl_->GetLogHandler();
}

bool Application::IsVREnabled() const {
    return impl_->IsVREnabled();
}
