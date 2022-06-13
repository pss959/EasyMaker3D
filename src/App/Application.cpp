#include "App/Application.h"

#include <algorithm>
#include <typeinfo>

#include <ion/gfxutils/shadermanager.h>
#include <ion/math/vectorutils.h>
#include <ion/text/fontmanager.h>

#include "App/ClickInfo.h"
#include "App/RegisterTypes.h"
#include "App/Renderer.h"
#include "App/SceneContext.h"
#include "Debug/Print.h"
#include "Enums/PrimitiveType.h"
#include "Executors/InitExecutors.h"
#include "Feedback/FindFeedback.h"
#include "Feedback/TooltipFeedback.h"
#include "Handlers/BoardHandler.h"
#include "Handlers/ControllerHandler.h"
#include "Handlers/InspectorHandler.h"
#include "Handlers/LogHandler.h"
#include "Handlers/MainHandler.h"
#include "Handlers/ShortcutHandler.h"
#include "Handlers/ViewHandler.h"
#include "IO/Reader.h"
#include "Items/Board.h"
#include "Items/BuildVolume.h"
#include "Items/Controller.h"
#include "Items/Inspector.h"
#include "Items/PrecisionControl.h"
#include "Items/RadialMenu.h"
#include "Items/Settings.h"
#include "Items/Shelf.h"
#include "Managers/ActionManager.h"
#include "Managers/AnimationManager.h"
#include "Managers/ClipboardManager.h"
#include "Managers/CommandManager.h"
#include "Managers/FeedbackManager.h"
#include "Managers/NameManager.h"
#include "Managers/PanelManager.h"
#include "Managers/PrecisionManager.h"
#include "Managers/SelectionManager.h"
#include "Managers/SessionManager.h"
#include "Managers/SettingsManager.h"
#include "Managers/TargetManager.h"
#include "Managers/ToolManager.h"
#include "Math/Animation.h"
#include "Math/Intersection.h"
#include "Math/Types.h"
#include "Models/Model.h"
#include "Panels/Panel.h"
#include "Panels/TreePanel.h"
#include "SG/Camera.h"
#include "SG/Change.h"
#include "SG/IonContext.h"
#include "SG/Node.h"
#include "SG/Scene.h"
#include "SG/Search.h"
#include "SG/ShaderProgram.h"
#include "SG/TextNode.h"
#include "SG/Tracker.h"
#include "SG/WindowCamera.h"
#include "Tests/TestContext.h"
#include "Tools/FindTools.h"
#include "Tools/Tool.h"
#include "Util/Assert.h"
#include "Util/Delay.h"
#include "Util/FilePath.h"
#include "Util/General.h"
#include "Util/KLog.h"
#include "Util/String.h"
#include "App/VRContext.h"
#include "Viewers/GLFWViewer.h"
#include "Viewers/VRViewer.h"
#include "Widgets/EdgeTargetWidget.h"
#include "Widgets/IconSwitcherWidget.h"
#include "Widgets/IconWidget.h"
#include "Widgets/PointTargetWidget.h"
#include "Widgets/PushButtonWidget.h"
#include "Widgets/Slider1DWidget.h"
#include "Widgets/StageWidget.h"

/// Set this to true to run without VR even if a headset is connected.
static const bool kIgnoreVR = false;

// ----------------------------------------------------------------------------
// Application::Loader_ class.
// ----------------------------------------------------------------------------

/// Application::Loader_ does most of the work of loading a scene. It manages
/// all context necessary to read the scene and set it up.
class Application::Loader_ {
  public:
    Loader_();

    /// Reads a scene from the given path. Returns a null SG::ScenePtr if
    /// anything fails.
    SG::ScenePtr LoadScene(const FilePath &path);

    const ion::gfxutils::ShaderManagerPtr & GetShaderManager() {
        return shader_manager_;
    }

    /// Returns the scene being read. This can be called before the scene
    /// loading is complete. If called afterwards, it returns the last scene
    /// read.
    const SG::ScenePtr GetScene() const { return scene_; }

  private:
    SG::TrackerPtr                  tracker_;
    ion::gfxutils::ShaderManagerPtr shader_manager_;
    ion::text::FontManagerPtr       font_manager_;
    SG::ScenePtr                    scene_;
    SG::IonContextPtr               ion_context_;
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

SG::ScenePtr Application::Loader_::LoadScene(const FilePath &path) {
    // Wipe out all previous shaders to avoid conflicts.
    shader_manager_.Reset(new ion::gfxutils::ShaderManager);
    ion_context_->Reset();
    ion_context_->SetShaderManager(shader_manager_);

    try {
        Reader reader;
        scene_ = reader.ReadScene(path, *tracker_);
        scene_->SetUpIon(ion_context_);
    }
    catch (std::exception &ex) {
        std::cerr << "*** Caught exception loading scene:\n"
                  << ex.what() << "\n";
        scene_.reset();
    }
    return scene_;
}

// ----------------------------------------------------------------------------
// Application::Impl_ class. This does most of the work for the Application
// class.
// ----------------------------------------------------------------------------

class  Application::Impl_ {
  public:
    Impl_();
    ~Impl_();

    void SetTestingFlag() { is_testing_ = true; }

    bool Init(const Vector2i &window_size, bool do_ion_remote);

    /// Returns true if VR is enabled (after Init() is called).
    bool IsVREnabled() const { return vr_context_.get(); }

    LogHandler & GetLogHandler() const { return *log_handler_; }

    /// Enters the main loop for the application.
    void MainLoop();

    /// Reloads the scene from its path, updating everything necessary.
    void ReloadScene();

    void GetTestContext(TestContext &tc);

  private:
    bool is_testing_ = false;

    std::unique_ptr<Loader_>  loader_;  ///< For loading and reloading scenes.

    /// \name Managers.
    ///@{
    ActionManagerPtr    action_manager_;
    AnimationManagerPtr animation_manager_;
    ClipboardManagerPtr clipboard_manager_;
    CommandManagerPtr   command_manager_;
    FeedbackManagerPtr  feedback_manager_;
    NameManagerPtr      name_manager_;
    PanelManagerPtr     panel_manager_;
    PrecisionManagerPtr precision_manager_;
    SelectionManagerPtr selection_manager_;
    SessionManagerPtr   session_manager_;
    SettingsManagerPtr  settings_manager_;
    TargetManagerPtr    target_manager_;
    ToolManagerPtr      tool_manager_;
    ///@}

    /// \name Various Contexts.
    ///@{
    SceneContextPtr            scene_context_;
    Tool::ContextPtr           tool_context_;
    std::unique_ptr<VRContext> vr_context_;
    ///@}

    /// \name Individual Handlers.
    ///@{
    BoardHandlerPtr      board_handler_;
    ControllerHandlerPtr controller_handler_;
    InspectorHandlerPtr  inspector_handler_;
    LogHandlerPtr        log_handler_;
    MainHandlerPtr       main_handler_;
    ShortcutHandlerPtr   shortcut_handler_;
    ViewHandlerPtr       view_handler_;
    ///@}

    /// \name Individual Viewers.
    ///@{
    GLFWViewerPtr    glfw_viewer_;
    VRViewerPtr      vr_viewer_;
    ///@}

    /// The renderer.
    RendererPtr      renderer_;

    /// All Handlers, in order.
    std::vector<HandlerPtr>    handlers_;

    /// All Viewers.
    std::vector<ViewerPtr>     viewers_;

    /// Registered Executor instances. These instances have to be held on to in
    /// this vector.
    std::vector<ExecutorPtr>   executors_;

    /// Executor::Context used to set up all Executor instances.
    Executor::ContextPtr       exec_context_;

    /// Panel::Context used to set up all Panel instances.
    Panel::ContextPtr          panel_context_;

    /// ActionManager::Context used to set up the ActionManager.
    ActionManager::ContextPtr  action_context_;

    /// All 3D icons that need to be updated every frame.
    std::vector<IconWidgetPtr> icons_;

    /// Special case for ToggleSpecializedToolIcon, which changes shape based
    /// on the current tool.
    IconSwitcherWidgetPtr      toggle_specialized_tool_icon_;

    /// Function invoked to show or hide a tooltip.
    Widget::TooltipFunc        tooltip_func_;

    /// Radius used for the stage, computed from the build volume size.
    float                      stage_radius_ = 1;

    /// Set to true when anything in the scene changes.
    bool                       scene_changed_ = true;

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

    /// Sets up a Tool::Context.
    void InitToolContext_();

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

    /// Shows the initial Panel.
    void ShowInitialPanel_();

    ///@}

    /// Sets up the given Widget to display a tooltip.
    void InitTooltip_(Widget &widget);

    void SelectionChanged_(const Selection &sel,
                           SelectionManager::Operation op);

    void SettingsChanged_(const Settings &settings);

    /// Updates enabled status and tooltips for all 3D icons.
    void UpdateIcons_();

    /// Updates global uniforms in the RootModel.
    void UpdateGlobalUniforms_();

    /// Handles all of the given events. Returns true if the application should
    /// keep running.
    bool HandleEvents_(std::vector<Event> &events, bool is_alternate_mode);

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

    /// Returns true if the Models in the scene should be visible. They are
    /// turned off during certain interactions.
    bool ShouldShowModels_() const;

    /// Returns a reasonable position for a tooltip for the given Widget whose
    /// size (in object coordinates) is provided.
    Vector3f ComputeTooltipTranslation_(Widget &widget,
                                        const Vector3f &size) const;

    /// Returns a rotation to make a tooltip face the camera.
    Rotationf ComputeTooltipRotation_() const;
};

// ----------------------------------------------------------------------------
// Application::Impl_ functions.
// ----------------------------------------------------------------------------

Application::Impl_::Impl_() : loader_(new Loader_) {
}

Application::Impl_::~Impl_() {
    handlers_.clear();
    viewers_.clear();

    // Instances must be destroyed in a particular order.
#if DEBUG
    Debug::ShutDown();
#endif
    view_handler_.reset();
    scene_context_.reset();
    renderer_.reset();
    vr_context_.reset();
    glfw_viewer_.reset();
}

bool Application::Impl_::Init(const Vector2i &window_size, bool do_ion_remote) {
    // Note that order here is extremely important!

    InitTypes_();

    // Make sure the scene loads properly and has all of the necessary items
    // (in the SceneContext) before doing anything else.
    scene_context_.reset(new SceneContext);
    const FilePath scene_path =
        FilePath::GetResourcePath("scenes", "workshop.mvn");
    SG::ScenePtr scene = loader_->LoadScene(scene_path);
    if (! scene)
        return false;
    scene_context_->FillFromScene(scene, true);
#if DEBUG
    Debug::SetSceneContext(scene_context_);
#endif

    if (! is_testing_) {
        // Set up the viewers. This also sets up the VRContext if VR is enabled
        // so that IsVREnabled() returns a valid value.
        if (! InitViewers_(window_size))
            return false;

        // Set up the renderer.
        const bool use_ion_remote = ! IsVREnabled() && do_ion_remote;
        renderer_.reset(
            new Renderer(loader_->GetShaderManager(), use_ion_remote));
        renderer_->Reset(*scene);
        if (IsVREnabled())
            vr_context_->InitRendering(*renderer_);
    }

    // This needs to exist for the ActionManager.
    tool_context_.reset(new Tool::Context);

    // Set up the tooltip function for use in all Widgets and Models.
    tooltip_func_ = [&](Widget &widget, const std::string &text, bool show){
        const std::string key = Util::ToString(&widget);
        if (show) {
            auto tf = feedback_manager_->ActivateWithKey<TooltipFeedback>(key);
            tf->SetText(text);
            auto tpath = SG::FindNodePathInScene(*scene_context_->scene, *tf);
            // Object coordinates for the tooltip object are the same as world
            // coordinates except for the scale applied to the tooltip object,
            // so just scale the size.
            tf->SetTranslation(
                ComputeTooltipTranslation_(widget,
                                           tf->GetScale() * tf->GetTextSize()));
            tf->SetRotation(ComputeTooltipRotation_());
        }
        else {
            feedback_manager_->DeactivateWithKey<TooltipFeedback>(key);
        }
    };

    InitHandlers_();
    InitManagers_();
    InitExecutors_();
    InitToolContext_();
    InitInteraction_();

    // Install things...
    main_handler_->SetPrecisionManager(precision_manager_);
    shortcut_handler_->SetActionManager(action_manager_);

    ConnectSceneInteraction_();
    ShowInitialPanel_();

    return true;
}

void Application::Impl_::MainLoop() {
    std::vector<Event> events;
    bool is_alternate_mode = false;
    bool keep_running = true;
    size_t render_count = 0;
    while (keep_running) {
        KLogger::SetRenderCount(render_count++);
        renderer_->BeginFrame();

        is_alternate_mode = glfw_viewer_->IsShiftKeyPressed();

        // Update the frustum used for intersection testing.
        scene_context_->frustum = glfw_viewer_->GetFrustum();

        // Update global uniforms in the RootModel.
        UpdateGlobalUniforms_();

        // Update everything that needs it.
        main_handler_->ProcessUpdate(is_alternate_mode);
        tool_context_->is_alternate_mode = is_alternate_mode;

        action_manager_->ProcessUpdate();

        // Process any animations. Do this after updating the MainHandler
        // because a click timeout may start an animation.
        const bool is_animating = animation_manager_->ProcessUpdate();

        // Enable or disable all icon widgets and update tooltips.
        UpdateIcons_();

        // Hide all the Models, Tools, etc. under certain conditions.
        scene_context_->model_hider->SetEnabled(ShouldShowModels_());

        // Always check for finished delayed threads.
        const bool is_any_delaying = Util::IsAnyDelaying();

        // Let the GLFWViewer know whether to poll events or wait for events.
        // If VR is active, it needs to continuously poll events to track the
        // headset and controllers properly. This means that the GLFWViewer
        // also needs to poll events (rather than wait for them) so as not to
        // block anything. The same is true if the MainHandler is in the middle
        // of handling something (not just waiting for events), if there is an
        // animation running, if something is being delayed, if something
        // changed in the scene, or if the user quit the app.
        const bool have_to_poll =
            IsVREnabled() || is_animating || is_any_delaying ||
            scene_changed_ || ! main_handler_->IsWaiting() ||
            action_manager_->ShouldQuit();
        glfw_viewer_->SetPollEventsFlag(have_to_poll);

        // Collect and handle all incoming events.
        events.clear();
        for (auto &viewer: viewers_)
            viewer->EmitEvents(events);
        keep_running = HandleEvents_(events, is_alternate_mode);

        // Update the TreePanel.
        scene_context_->tree_panel->SetSessionString(
            session_manager_->GetSessionString());

        // Clear this flag before rendering. Rendering might cause some changes
        // to occur, and those may need to be detected.
        scene_changed_ = false;

        // Render to all viewers.
        for (auto &viewer: viewers_) {
            KLOG('R', "Render to " << Util::Demangle(typeid(*viewer).name()));
            viewer->Render(*scene_context_->scene, *renderer_);
        }

        renderer_->EndFrame();
    }

    // No longer running; exit VR.
    if (vr_context_)
        vr_context_->Shutdown();
}

void Application::Impl_::ReloadScene() {
    ASSERT(scene_context_);
    ASSERT(scene_context_->scene);

    name_manager_->Reset();
    panel_manager_->Reset();
    selection_manager_->Reset();
    command_manager_->ResetCommandList();
    tool_manager_->ClearTools();
    Model::ResetColors();

    // Reset all handlers that may be holding onto state.
    for (auto &handler: handlers_)
        handler->Reset();

    // Wipe out any events that may be pending in viewers.
    for (auto &viewer: viewers_)
        viewer->FlushPendingEvents();

    try {
        SG::ScenePtr scene =
            loader_->LoadScene(scene_context_->scene->GetPath());
        scene_context_->FillFromScene(scene, true);
        ConnectSceneInteraction_();
        view_handler_->ResetView();
        renderer_->Reset(*scene);
        ShowInitialPanel_();
    }
    catch (std::exception &ex) {
        std::cerr << "*** Caught exception reloading scene:\n"
                  << ex.what() << "\n";
    }
}

void Application::Impl_::GetTestContext(TestContext &tc) {
    // This should not be called before Init().
    ASSERT(session_manager_);
    ASSERT(scene_context_);
    tc.command_manager = command_manager_;
    tc.session_manager = session_manager_;
    tc.scene_context   = scene_context_;
}

void Application::Impl_::InitTypes_() {
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
    if (! kIgnoreVR && vr_context_->InitSystem()) {
        vr_viewer_.reset(new VRViewer(*vr_context_));
        viewers_.push_back(vr_viewer_);
    }
    else {
        vr_context_.reset();
    }
    return true;
}

void Application::Impl_::InitHandlers_() {
    board_handler_.reset(new BoardHandler);
    controller_handler_.reset(new ControllerHandler);
    inspector_handler_.reset(new InspectorHandler);
    log_handler_.reset(new LogHandler);
    shortcut_handler_.reset(new ShortcutHandler);
    view_handler_.reset(new ViewHandler());
    main_handler_.reset(new MainHandler);

    // Order here is extremely important, since Handlers are passed events in
    // this order.

    // LogHandler has to be first so it can log all events.
    handlers_.push_back(log_handler_);

    // ControllerHandler just updates controller position, so it needs all
    // controller events.
    if (IsVREnabled())
        handlers_.push_back(controller_handler_);

    // InspectorHandler traps most events when active.
    handlers_.push_back(inspector_handler_);

    // Board Handler needs to process keyboard events before anything else.
    ASSERT(scene_context_);
    ASSERT(scene_context_->floating_board);
    handlers_.push_back(board_handler_);

    handlers_.push_back(shortcut_handler_);
    handlers_.push_back(view_handler_);

    // MainHandler does most of the work.
    handlers_.push_back(main_handler_);

#if DEBUG
    Debug::SetLogHandler(log_handler_);
#endif
}

void Application::Impl_::InitManagers_() {
    ASSERT(main_handler_);
    ASSERT(tool_context_);

    animation_manager_.reset(new AnimationManager);
    clipboard_manager_.reset(new ClipboardManager);
    feedback_manager_.reset(new FeedbackManager);
    command_manager_.reset(new CommandManager);
    name_manager_.reset(new NameManager);
    panel_manager_.reset(new PanelManager);
    precision_manager_.reset(new PrecisionManager);
    selection_manager_.reset(new SelectionManager);
    settings_manager_.reset(new SettingsManager);
    target_manager_.reset(new TargetManager(command_manager_));
    tool_manager_.reset(new ToolManager(*target_manager_));

    settings_manager_->SetChangeFunc(
        [&](const Settings &settings){ SettingsChanged_(settings); });

    // The ActionManager requires its own context.
    action_context_.reset(new ActionManager::Context);
    action_context_->scene_context     = scene_context_;
    action_context_->tool_context      = tool_context_;
    action_context_->clipboard_manager = clipboard_manager_;
    action_context_->command_manager   = command_manager_;
    action_context_->name_manager      = name_manager_;
    action_context_->panel_manager     = panel_manager_;
    action_context_->precision_manager = precision_manager_;
    action_context_->selection_manager = selection_manager_;
    action_context_->settings_manager  = settings_manager_;
    action_context_->target_manager    = target_manager_;
    action_context_->tool_manager      = tool_manager_;
    action_context_->main_handler      = main_handler_;
    action_manager_.reset(new ActionManager(action_context_));
    action_manager_->SetReloadFunc([&]() { ReloadScene(); });

    session_manager_.reset(new SessionManager(action_manager_, command_manager_,
                                              selection_manager_));

#if DEBUG
    Debug::SetCommandList(command_manager_->GetCommandList());
#endif
}

void Application::Impl_::InitExecutors_() {
    ASSERT(command_manager_);

    executors_ = InitExecutors();

    exec_context_.reset(new Executor::Context);
    exec_context_->animation_manager = animation_manager_;
    exec_context_->clipboard_manager = clipboard_manager_;
    exec_context_->command_manager   = command_manager_;
    exec_context_->name_manager      = name_manager_;
    exec_context_->selection_manager = selection_manager_;
    exec_context_->settings_manager  = settings_manager_;
    exec_context_->target_manager    = target_manager_;
    exec_context_->tooltip_func      = tooltip_func_;
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
            scene_context_->stage->ApplyScaleChange(.1f * value);
    };

    main_handler_->GetValuatorChanged().AddObserver(this, scroll);
    main_handler_->GetClicked().AddObserver(
        this, [&](const ClickInfo &info){ ProcessClick_(info); });

    // Detect selection changes to update the ToolManager.
    selection_manager_->GetSelectionChanged().AddObserver(
        this, [&](const Selection &sel, SelectionManager::Operation op){
            SelectionChanged_(sel, op); });
}

void Application::Impl_::InitToolContext_() {
    ASSERT(tool_manager_);
    ASSERT(tool_context_);

    tool_context_->command_manager   = command_manager_;
    tool_context_->feedback_manager  = feedback_manager_;
    tool_context_->panel_manager     = panel_manager_;
    tool_context_->precision_manager = precision_manager_;
    tool_context_->settings_manager  = settings_manager_;
    tool_context_->target_manager    = target_manager_;
}

void Application::Impl_::ConnectSceneInteraction_() {
    ASSERT(scene_context_);
    ASSERT(scene_context_->scene);

    auto &scene = *scene_context_->scene;

    // Tell the ActionManager::Context about the new Scene.
    action_context_->scene_context = scene_context_;

    // Tell the SelectionManager and Executor::Context about the new RootModel.
    selection_manager_->SetRootModel(scene_context_->root_model);
    exec_context_->root_model = scene_context_->root_model;

    // Set up the Panel::Context and let the PanelManager find the new Panel
    // instances.
    if (! panel_context_) {
        panel_context_.reset(new Panel::Context);
        panel_context_->name_manager      = name_manager_;
        panel_context_->selection_manager = selection_manager_;
        panel_context_->session_manager   = session_manager_;
        panel_context_->settings_manager  = settings_manager_;
        panel_context_->panel_helper      = panel_manager_;
    }
    panel_manager_->FindPanels(scene, panel_context_);

    // The TreePanel does not go through the PanelManager, so set it up.
    scene_context_->tree_panel->SetContext(panel_context_);

    inspector_handler_->SetInspector(scene_context_->inspector);

    board_handler_->AddBoard(scene_context_->floating_board);
    board_handler_->AddBoard(scene_context_->tool_board);
    main_handler_->SetSceneContext(scene_context_);
    target_manager_->SetPathToStage(scene_context_->path_to_stage);

    // Inform the viewers and ViewHandler about the cameras in the scene.
    if (! is_testing_) {
        view_handler_->SetCamera(scene_context_->window_camera);
        glfw_viewer_->SetCamera(scene_context_->window_camera);
        if (IsVREnabled())
            vr_viewer_->SetCamera(scene_context_->vr_camera);
    }

    // Set Nodes in the Controllers.
    auto lc = scene_context_->left_controller;
    auto rc = scene_context_->right_controller;
    ASSERT(lc);
    ASSERT(rc);
    lc->SetHand(Hand::kLeft);
    rc->SetHand(Hand::kRight);
    controller_handler_->SetControllers(lc, rc);

    // Enable or disable controllers.
    lc->SetEnabled(IsVREnabled());
    rc->SetEnabled(IsVREnabled());

    // Hook up the height slider.
    scene_context_->height_slider->GetValueChanged().AddObserver(
        this, [&](Widget &w, const float &val){
            scene_context_->gantry->SetHeight(Lerp(val, -10.f, 100.f)); });
    InitTooltip_(*scene_context_->height_slider);

    // Detect changes in the scene.
    scene.GetRootNode()->GetChanged().AddObserver(
        this, [this](SG::Change change, const SG::Object &obj){
            KLOG('N', "Application got change " << Util::EnumName(change)
                 << " from " << obj.GetDesc());
            scene_changed_ = true;
        });

    // Set up the Boards.
    AddBoards_();

    // Add all Tools from the templates in the scene.
    AddTools_();

    // Add all Feedback instances to the manager.
    AddFeedback_();

    // Set up 3D icons on the shelves. Note that this requires the camera to be
    // in the correct position (above).
    AddIcons_();

    // Add all Grippable objects to the MainHandler.
    main_handler_->AddGrippable(scene_context_->floating_board);
    main_handler_->AddGrippable(tool_manager_);

    // Set up targets in the TargetManager.
    target_manager_->InitTargets(
        SG::FindTypedNodeInScene<PointTargetWidget>(scene, "PointTarget"),
        SG::FindTypedNodeInScene<EdgeTargetWidget>(scene, "EdgeTarget"));

    // Hook up the exit sign.
    auto exit_sign =
        SG::FindTypedNodeInScene<PushButtonWidget>(scene, "ExitSign");
    exit_sign->GetClicked().AddObserver(
        this, [this](const ClickInfo &){
            action_manager_->ApplyAction(Action::kQuit);
        });
    InitTooltip_(*exit_sign);

    // Set up the TreePanel.
    auto wall_board = SG::FindTypedNodeInScene<Board>(scene, "WallBoard");
    wall_board->SetPanel(scene_context_->tree_panel);
    wall_board->SetPanelScale(Defaults::kPanelToWorld * 4);  // Far away.
    wall_board->Show(true);

    // Set up the radial menus.
    auto apply = [&](size_t index, Action action){
        if (action_manager_->CanApplyAction(action))
            action_manager_->ApplyAction(action);
    };
    scene_context_->left_radial_menu->GetButtonClicked().AddObserver(
        this, apply);
    scene_context_->right_radial_menu->GetButtonClicked().AddObserver(
        this, apply);

    // Now that everything has been found, disable searching through the
    // "Definitions" Node.
    SG::FindNodeInScene(scene, "Definitions")->SetFlagEnabled(
        SG::Node::Flag::kSearch, false);

    // Simulate a change in settings to update everything.
    SettingsChanged_(settings_manager_->GetSettings());
}

void Application::Impl_::AddTools_() {
    ASSERT(tool_manager_);
    ASSERT(scene_context_);
    ASSERT(scene_context_->scene);

    tool_context_->root_model = scene_context_->root_model;

    SG::Scene &scene = *scene_context_->scene;
    auto path_to_parent = SG::FindNodePathInScene(scene, "ToolParent");
    tool_manager_->ClearTools();
    tool_manager_->SetParentNode(path_to_parent.back());
    tool_context_->path_to_parent_node = path_to_parent;

    // Find the parent node containing all Tools in the scene and use it to get
    // all Tool instances.
    const std::vector<ToolPtr> tools = FindTools(scene);
    for (auto &tool: tools)
        tool->SetContext(tool_context_);
    tool_manager_->AddTools(tools);
    tool_manager_->SetDefaultGeneralTool("TranslationTool");
}

void Application::Impl_::AddFeedback_() {
    ASSERT(scene_context_);
    ASSERT(scene_context_->scene);
    ASSERT(feedback_manager_);

    SG::Scene &scene = *scene_context_->scene;

    const SG::NodePtr world_fb_parent =
        SG::FindNodeInScene(scene, "WorldFeedbackParent");
    const SG::NodePtr stage_fb_parent =
        SG::FindNodeInScene(scene, "StageFeedbackParent");
    feedback_manager_->Reset();
    feedback_manager_->SetParentNodes(world_fb_parent, stage_fb_parent);
    feedback_manager_->SetSceneBoundsFunc([this](){
        return scene_context_->root_model->GetBounds(); });
    feedback_manager_->SetPathToStage(scene_context_->path_to_stage);

    for (auto &fb: FindFeedback(*scene.GetRootNode()))
        feedback_manager_->AddOriginal<Feedback>(fb);
}

void Application::Impl_::AddIcons_() {
    ASSERT(action_manager_);
    ASSERT(scene_context_);
    ASSERT(scene_context_->scene);

    icons_.clear();

    Point3f cam_pos;
    if (is_testing_) {
        cam_pos = Point3f(0, 0, 100);
    }
    else {
        ASSERT(glfw_viewer_);
        cam_pos = glfw_viewer_->GetFrustum().position;
    }

    // Set up the icons on the shelves.
    SG::Scene &scene = *scene_context_->scene;
    const SG::NodePtr shelves = SG::FindNodeInScene(scene, "Shelves");
    for (const auto &child: shelves->GetChildren()) {
        const ShelfPtr shelf = Util::CastToDerived<Shelf>(child);
        ASSERT(shelf);
        shelf->LayOutIcons(cam_pos);
        Util::AppendVector(shelf->GetIcons(), icons_);
    }

    // PrecisionControl is a special case.
    auto prec_control =
        SG::FindTypedNodeInScene<PrecisionControl>(scene, "PrecisionControl");
    Util::AppendVector(prec_control->GetIcons(), icons_);

    // Set up enabling, actions, and tooltips for all icons.
    for (const auto &icon: icons_) {
        icon->SetEnableFunction([&](){
            return action_manager_->CanApplyAction(icon->GetAction()); });
        icon->GetClicked().AddObserver(
            this, [&](const ClickInfo &){
                action_manager_->ApplyAction(icon->GetAction());});
        InitTooltip_(*icon);
    }

    // Store the ToggleSpecializedToolIcon so it can be updated properly.
    toggle_specialized_tool_icon_ =
        SG::FindTypedNodeInScene<IconSwitcherWidget>(
            scene, "ToggleSpecializedToolIcon");
}

void Application::Impl_::AddBoards_() {
    ASSERT(scene_context_);
    ASSERT(scene_context_->floating_board);
    ASSERT(scene_context_->tool_board);

    tool_context_->board = scene_context_->tool_board;

    const auto &fb = scene_context_->floating_board;
    panel_manager_->SetDefaultBoard(fb);
    fb->SetTranslation(Vector3f(0, 14, 0));
    fb->Show(true);

    // Install a path filter in the MainHandler that disables interaction with
    // other widgets when the FloatingBoard is visible.
    ASSERT(main_handler_);
    auto filter = [fb](const SG::NodePath &path){
        return ! fb->IsShown() || Util::Contains(path, fb);
    };
    main_handler_->SetPathFilter(filter);
}

void Application::Impl_::ShowInitialPanel_() {
    // Show the SessionPanel.
    action_manager_->ApplyAction(Action::kOpenSessionPanel);
}

void Application::Impl_::InitTooltip_(Widget &widget) {
    ASSERT(tooltip_func_);
    widget.SetTooltipFunc(tooltip_func_);
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
    scene_context_->tree_panel->ModelsChanged();
}

void Application::Impl_::SettingsChanged_(const Settings &settings) {
    TooltipFeedback::SetDelay(settings.GetTooltipDelay());

    scene_context_->left_radial_menu->UpdateFromInfo(
        settings.GetLeftRadialMenuInfo());
    scene_context_->right_radial_menu->UpdateFromInfo(
        settings.GetRightRadialMenuInfo());


    /// Update the build volume size.
    const auto &bv_size = settings.GetBuildVolumeSize();
    scene_context_->build_volume->SetSize(bv_size);

    /// Update the stage radius based on the build volume size.
    const float old_stage_radius = stage_radius_;
    stage_radius_ = .8f * std::max(bv_size[0], bv_size[2]);
    if (stage_radius_ != old_stage_radius)
        scene_context_->stage->SetStageRadius(stage_radius_);
}

void Application::Impl_::UpdateIcons_() {
    for (auto &icon: icons_) {
        ASSERT(icon);

        const bool enabled = icon->ShouldBeEnabled();
        icon->SetInteractionEnabled(enabled);

        if (enabled) {
            const Action action = icon->GetAction();
            icon->SetTooltipText(action_manager_->GetActionTooltip(action));
            if (icon->IsToggle())
                icon->SetToggleState(action_manager_->GetToggleState(action));
        }
    }

    // Special case for the ToggleSpecializedToolIcon.
    const auto &sel = selection_manager_->GetSelection();
    const auto tool = tool_manager_->GetSpecializedToolForSelection(sel);
    const std::string tool_name = tool ? tool->GetTypeName() : "Null";
    toggle_specialized_tool_icon_->SetIndexByName(tool_name + "Icon");
    toggle_specialized_tool_icon_->SetToggleState(
        tool_manager_->IsUsingSpecializedTool());
}

void Application::Impl_::UpdateGlobalUniforms_() {
    // Get the current world-to-stage matrix.
    const auto wsm =
        CoordConv(scene_context_->path_to_stage).GetRootToObjectMatrix();

    // Get the build volume size. Note that an inactive build volume is
    // indicated by a zero size. The uniform for the active build volume size
    // has to be scaled to match the stage.
    const auto &bv = *scene_context_->build_volume;
    const Vector3f bv_size =
        bv.IsActive() ? scene_context_->stage->GetRadiusScale() * bv.GetSize() :
        Vector3f::Zero();

    // Update the uniforms.
    scene_context_->root_model->UpdateGlobalUniforms(wsm, bv_size);
}

bool Application::Impl_::HandleEvents_(std::vector<Event> &events,
                                       bool is_alternate_mode) {
    for (auto &event: events) {
        event.is_alternate_mode = is_alternate_mode;

        // Special case for exit events.
        if (event.flags.Has(Event::Flag::kExit))
            return false;

        for (auto &handler: handlers_) {
            if (handler->IsEnabled() && handler->HandleEvent(event)) {
                KLOG('e', "Event handled by "
                     << Util::Demangle(typeid(*handler).name()));
                break;
            }
        }
    }

    // Also check for action resulting in quitting.
    return ! action_manager_->ShouldQuit();
}

void Application::Impl_::ProcessClick_(const ClickInfo &info) {
    // Shorthand for selecting the given Model from the ClickInfo's Hit path.
    auto select_model = [&](const Model &model, bool is_alt){
        const SelPath path(info.hit.path.GetSubPath(model));
        selection_manager_->ChangeModelSelection(path, is_alt);
    };

    KLOG('k', "Click on widget "
         << info.widget << " is_alt = " << info.is_alternate_mode
         << " is_long = " << info.is_long_press);
    if (info.widget) {
        if (info.widget == scene_context_->stage.get()) {
            // Reset the stage if alt-clicked.
            if (info.is_alternate_mode) {
                const Vector3f  scale = scene_context_->stage->GetScale();
                const Rotationf rot   = scene_context_->stage->GetRotation();
                animation_manager_->StartAnimation(
                    [&, scale, rot](float t){
                    return ResetStage_(scale, rot, t); });
            }
        }
        else if (info.widget == scene_context_->height_slider.get()) {
            // Reset the height slider if clicked or alt-clicked.
            const float     height = scene_context_->height_slider->GetValue();
            const Rotationf orient =
                scene_context_->window_camera->GetOrientation();
            const bool      reset_view = info.is_alternate_mode;
            animation_manager_->StartAnimation(
                [&, height, orient, reset_view](float t){
                return ResetHeightAndView_(height, orient, reset_view, t); });
        }
        else {
            if (info.widget->IsInteractionEnabled()) {
                info.widget->Click(info);
                // If long pressing on a Model, select and inspect it.
                if (info.is_long_press) {
                    auto model = dynamic_cast<Model *>(info.widget);
                    if (model) {
                        select_model(*model, false);
                        // Make sure that the action is enabled.
                        action_manager_->ProcessUpdate();
                        action_manager_->ApplyAction(Action::kToggleInspector);
                    }
                }
            }
        }
    }
    // If the intersected object is part of a Tool, process the click as if
    // it were a click on the attached Model.
    else if (ToolPtr tool = info.hit.path.FindNodeUpwards<Tool>()) {
        ASSERT(tool->GetModelAttachedTo());
        select_model(*tool->GetModelAttachedTo(), info.is_alternate_mode);
    }
    // If the intersected object is part of a Board, ignore the click.
    else if (info.hit.path.FindNodeUpwards<Board>()) {
        ;  // Do nothing.
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
    const Anglef angle = AbsAngle(RotationAngle(start_rot));
    const float max_scale = start_scale[GetMaxAbsElementIndex(start_scale)];
    const float duration = std::max(angle.Degrees() / kMaxDeltaAngle,
                                    max_scale / kMaxDeltaScale);

    // Interpolate and update the stage's scale and rotation.
    const float t = std::min(1.f, time / duration);
    StageWidget &stage = *scene_context_->stage;
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
        const Anglef angle = AbsAngle(RotationAngle(start_view_rot));
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

bool Application::Impl_::ShouldShowModels_() const {
    // Hide Models if the FloatingBoard, Inspector, or VirtualKeyboard is
    // visible.
    /// \todo Add VirtualKeyboard
    return ! (scene_context_->floating_board->IsShown() ||
              scene_context_->inspector->IsEnabled());
}

Vector3f Application::Impl_::ComputeTooltipTranslation_(
    Widget &widget, const Vector3f &world_size) const {
    // Find a path to the Widget.
    auto path = SG::FindNodePathInScene(*scene_context_->scene, widget);

    // Convert its location to world coordinates.
    const Point3f world_pt = CoordConv(path).ObjectToRoot(Point3f::Zero());

    // Use a plane at a reasonable distance past the image plane of the
    // frustum.
    const auto &frustum = scene_context_->frustum;
    const Point3f  &cam_pos = frustum.position;
    const Vector3f  cam_dir = frustum.orientation * -Vector3f::AxisZ();
    const Plane plane(cam_pos + Defaults::kTooltipDistance * cam_dir, cam_dir);

    auto intersect_plane = [&](const Ray &ray){
        float distance;
        RayPlaneIntersect(ray, plane, distance);
        return ray.GetPoint(distance);
    };

    // Find the position of the widget in this plane.
    Point3f position = intersect_plane(
        Ray(cam_pos, ion::math::Normalized(world_pt - cam_pos)));

    // Get the lower-left and upper-right frustum corners in this plane.
    const Point3f ll = intersect_plane(frustum.BuildRay(Point2f(0, 0)));
    const Point3f ur = intersect_plane(frustum.BuildRay(Point2f(1, 1)));

    // Make sure the tooltip is within a reasonable margin of each edge.
    const float kMargin = .05f;
    const Vector2f half_size = .5f * Vector2f(world_size[0], world_size[1]);
    position[0] = Clamp(position[0],
                        ll[0] + kMargin + half_size[0],
                        ur[0] - kMargin - half_size[0]);
    position[1] = Clamp(position[1],
                        ll[1] + kMargin + half_size[1],
                        ur[1] - kMargin - half_size[1]);

    // Move the tooltip up a little to be away from the pointer.
    position[1] += world_size[1];

    return Vector3f(position);
}

Rotationf Application::Impl_::ComputeTooltipRotation_() const {
    // Rotate to face the camera.
    return scene_context_->frustum.orientation;
}

// ----------------------------------------------------------------------------
// Application functions.
// ----------------------------------------------------------------------------

Application::Application() : impl_(new Impl_) {
}

Application::~Application() {
}

bool Application::Init(const Vector2i &window_size, bool do_ion_remote) {
    return impl_->Init(window_size, do_ion_remote);
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

void Application::SetTestingFlag() {
    impl_->SetTestingFlag();
}

void Application::GetTestContext(TestContext &tc) {
    impl_->GetTestContext(tc);
}
