#include "App/Application.h"

#include <algorithm>
#include <typeinfo>

#include <ion/gfxutils/shadermanager.h>
#include <ion/math/vectorutils.h>

#include "App/RegisterTypes.h"
#include "App/SceneContext.h"
#include "App/SceneLoader.h"
#include "Base/IEmitter.h"
#include "Base/VirtualKeyboard.h"
#include "Debug/Shortcuts.h"
#include "Enums/PrimitiveType.h"
#include "Executors/InitExecutors.h"
#include "Feedback/FindFeedback.h"
#include "Feedback/TooltipFeedback.h"
#include "Handlers/BoardHandler.h"
#include "Handlers/ControllerHandler.h"
#include "Handlers/DragRectHandler.h"
#include "Handlers/InspectorHandler.h"
#include "Handlers/LogHandler.h"
#include "Handlers/MainHandler.h"
#include "Handlers/ShortcutHandler.h"
#include "Handlers/ViewHandler.h"
#include "Items/BuildVolume.h"
#include "Items/Controller.h"
#include "Items/Inspector.h"
#include "Items/PrecisionControl.h"
#include "Items/RadialMenu.h"
#include "Items/Settings.h"
#include "Items/Shelf.h"
#include "Managers/ActionManager.h"
#include "Managers/AnimationManager.h"
#include "Managers/BoardManager.h"
#include "Managers/ClipboardManager.h"
#include "Managers/CommandManager.h"
#include "Managers/EventManager.h"
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
#include "Panels/Board.h"
#include "Panels/DialogPanel.h"
#include "Panels/KeyboardPanel.h"
#include "Panels/Panel.h"
#include "Panels/TreePanel.h"
#include "Place/ClickInfo.h"
#include "SG/Change.h"
#include "SG/IonContext.h"
#include "SG/Node.h"
#include "SG/Scene.h"
#include "SG/Search.h"
#include "SG/ShaderProgram.h"
#include "SG/TextNode.h"
#include "SG/VRCamera.h"
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
#include "Util/Tuning.h"
#include "VR/VRContext.h"
#include "Viewers/GLFWViewer.h"
#include "Viewers/Renderer.h"
#include "Viewers/VRViewer.h"
#include "Widgets/EdgeTargetWidget.h"
#include "Widgets/IconSwitcherWidget.h"
#include "Widgets/IconWidget.h"
#include "Widgets/PointTargetWidget.h"
#include "Widgets/PushButtonWidget.h"
#include "Widgets/Slider1DWidget.h"
#include "Widgets/StageWidget.h"

// ----------------------------------------------------------------------------
// Application::Impl_ class. This does most of the work for the Application
// class.
// ----------------------------------------------------------------------------

class  Application::Impl_ {
  public:
    Impl_();
    ~Impl_();

    bool Init(const Application::Options &options);

    /// Returns true if VR is enabled (after Init() is called).
    bool IsVREnabled() const { return vr_context_.get(); }

    LogHandler & GetLogHandler() const { return *log_handler_; }

    bool ProcessFrame(size_t render_count, bool force_poll);

    /// Reloads the scene from its path, updating everything necessary.
    void ReloadScene();

    void SaveCrashSession(const FilePath &path, const std::string &message,
                          const std::vector<std::string> &stack);

    void Shutdown() { if (IsVREnabled()) vr_context_->Shutdown(); }

    void SetTestingFlag() { is_testing_ = true; }
    void GetTestContext(TestContext &tc);
    void SetAskBeforeQuitting(bool ask) { ask_before_quitting_ = ask; }
    void AddEmitter(const IEmitterPtr &emitter);
    Vector2i GetWindowSize() const;
    Renderer & GetRenderer() { return *renderer_; }
    void ForceTouchMode(bool is_on) { force_touch_mode_on_ = is_on; }
    void EnableMouseMotionEvents(bool enable) {
        glfw_viewer_->EnableMouseMotionEvents(enable);
    }
    void SetControllerRenderOffsets(const Vector3f &l_offset,
                                    const Vector3f &r_offset) {
        controller_handler_->SetRenderOffsets(l_offset, r_offset);
    }

  private:
    /// Run states for the main loop.
    enum class RunState_ {
        kRunning,        ///< Running (normal state).
        kQuitRequested,  ///< Processing a request to quit the app.
        kQuitting,       ///< Actually quitting the app.
    };

    /// Saves Options passed to Init().
    Application::Options options_;

    bool is_testing_          = false;
    bool ask_before_quitting_ = true;

    std::unique_ptr<SceneLoader> loader_;

    /// \name Managers.
    ///@{
    ActionManagerPtr    action_manager_;
    AnimationManagerPtr animation_manager_;
    BoardManagerPtr     board_manager_;
    ClipboardManagerPtr clipboard_manager_;
    CommandManagerPtr   command_manager_;
    EventManagerPtr     event_manager_;
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
#if ENABLE_DEBUG_FEATURES
    DragRectHandlerPtr   drag_rect_handler_;
#endif
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

    /// All viewers.
    std::vector<ViewerPtr>     viewers_;

    /// All event emitters.
    std::vector<IEmitterPtr>   emitters_;

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

    /// VirtualKeyboard; non-null only when VR is enabled.
    VirtualKeyboardPtr         virtual_keyboard_;

    /// Special case for ToggleSpecializedToolIcon, which changes shape based
    /// on the current tool.
    IconSwitcherWidgetPtr      toggle_specialized_tool_icon_;

    /// Function invoked to show or hide a tooltip.
    Widget::TooltipFunc        tooltip_func_;

    /// Set to true when anything in the scene changes.
    bool                       scene_changed_ = true;

    /// Current state of running the application.
    RunState_                  run_state_ = RunState_::kRunning;

    /// When true, touch mode is always on.
    bool                       force_touch_mode_on_ = false;

    /// \name One-time Initialization
    /// Each of these functions sets up items that are needed by the
    /// application. They are called one time only.
    ///@{

    /// Sets up SG and registers all types.
    void InitTypes_();

    /// Initializes the GLFWViewer and optionally the VRViewer, adding them to
    /// the viewers_ vector. Returns false if the GLFWViewer could not be set
    /// up.
    bool InitViewers_();

    /// Initializes all Handlers.
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

    /// If possible, replaces the controller model for the given hand with one
    /// supplied by the VRContext.
    void ReplaceControllerModel_(Hand hand);

    /// Adds the Board instances for 2D-ish UI.
    void AddBoards_();

    /// Adds templates for all Tools to the ToolManager.
    void AddTools_();

    /// Adds templates for all Feedback types to the FeedbackManager.
    void AddFeedback_();

    /// Adds the 3D icons on the shelves.
    void AddIcons_();

    /// Initializes and adds all Grippable instances to the MainHandler.
    void AddGrippables_();

    /// Initializes the radial menus - if in VR, attaches them to the
    /// controllers properly.
    void InitRadialMenus_();

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

    /// If there are changes to the session, this opens a DialogPanel to ask
    /// the user whether to quit or not. Sets run_state_ appropriately.
    void TryQuit_();

    /// Emits and handles all events. Returns true if the application should
    /// keep running.
    bool ProcessEvents_(bool is_modified_mode, bool force_poll);

    /// Processes a click on something in the scene.
    void ProcessClick_(const ClickInfo &info);

    /// Initiates animation to reset the stage.
    void StartResetStage_(bool is_modified_mode);

    /// Initiates animation to reset the height slider.
    void StartResetHeight_(bool is_modified_mode);

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

Application::Impl_::Impl_() : loader_(new SceneLoader) {
}

Application::Impl_::~Impl_() {
    event_manager_->ClearHandlers();
    viewers_.clear();
    emitters_.clear();

    // Instances must be destroyed in a particular order.
#if ENABLE_DEBUG_FEATURES
    Debug::ShutDown();
#endif
    view_handler_.reset();
    scene_context_.reset();
    renderer_.reset();
    vr_context_.reset();
    glfw_viewer_.reset();
}

bool Application::Impl_::Init(const Application::Options &options) {
    options_ = options;

    // Note that order here is extremely important!

    InitTypes_();

    // Make sure the scene loads properly and has all of the necessary items
    // (in the SceneContext) before doing anything else.
    scene_context_.reset(new SceneContext);
    const FilePath scene_path =
        FilePath::GetResourcePath("scenes", "workshop" + TK::kDataFileSuffix);
    SG::ScenePtr scene = loader_->LoadScene(scene_path);
    if (! scene)
        return false;
    scene_context_->FillFromScene(scene, true);
#if ENABLE_DEBUG_FEATURES
    Debug::SetSceneContext(scene_context_);
#endif

    if (! is_testing_) {
        // Set up the viewers. This also sets up the VRContext if VR is enabled
        // so that IsVREnabled() returns a valid value.
        if (! InitViewers_())
            return false;

        // Set up the renderer.
        const bool use_ion_remote = options_.do_ion_remote;
        renderer_.reset(
            new Renderer(loader_->GetShaderManager(), use_ion_remote));
        renderer_->Reset(*scene);
        if (IsVREnabled())
            vr_context_->InitRendering(*renderer_);
        virtual_keyboard_.reset(new VirtualKeyboard);
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
    if (options.show_session_panel)
        ShowInitialPanel_();

    // Tell the ActionManager how to quit.
    action_manager_->SetQuitFunc([&]{ TryQuit_(); });

    return true;
}

bool Application::Impl_::ProcessFrame(size_t render_count, bool force_poll) {
    ASSERT(run_state_ != RunState_::kQuitting);

    KLogger::SetRenderCount(render_count++);
    renderer_->BeginFrame();

    const bool is_modified_mode = glfw_viewer_->IsShiftKeyPressed();

    // Update the frustum used for intersection testing.
    scene_context_->frustum = glfw_viewer_->GetFrustum();

    // Update global uniforms in the RootModel.
    UpdateGlobalUniforms_();

    // Update everything that needs it.
    main_handler_->SetTouchable(board_manager_->GetCurrentBoard());
    main_handler_->ProcessUpdate(is_modified_mode);
    tool_context_->is_modified_mode = is_modified_mode;

    action_manager_->ProcessUpdate();

    // Process any animations. Do this after updating the MainHandler
    // because a click timeout may start an animation.
    animation_manager_->ProcessUpdate();

    // Enable or disable all icon widgets and update tooltips.
    UpdateIcons_();

    // Hide all the Models, Tools, etc. under certain conditions.
    scene_context_->work_hider->SetEnabled(ShouldShowModels_());

    // Put controllers in touch mode if the AppBoard, KeyBoard, or
    // ToolBoard is active.
    const bool in_touch_mode =
        force_touch_mode_on_ ||
        scene_context_->app_board->IsShown() ||
        scene_context_->key_board->IsShown() ||
        scene_context_->tool_board->IsShown();
    scene_context_->left_controller->SetTouchMode(in_touch_mode);
    scene_context_->right_controller->SetTouchMode(in_touch_mode);

    // Emit and process Events. This returns false if the application
    // should quit because the window was closed.
    if (! ProcessEvents_(is_modified_mode, force_poll))
        TryQuit_();

    // Update the TreePanel.
    scene_context_->tree_panel->SetSessionString(
        session_manager_->GetSessionString());

    // Update the current tool if there is one and it is attached to a Model.
    // Do this after processing events so the world is up to date.
    auto tool = tool_manager_->GetCurrentTool();
    if (tool && tool->GetModelAttachedTo())
        tool->Update();

    // Clear this flag before rendering. Rendering might cause some changes
    // to occur, and those may need to be detected.
    scene_changed_ = false;

    // Render to all viewers.
    for (auto &viewer: viewers_) {
        KLOG('R', "Render to " << Util::Demangle(typeid(*viewer).name()));
        viewer->Render(*scene_context_->scene, *renderer_);
    }

    renderer_->EndFrame();

    return run_state_ != RunState_::kQuitting;
}

void Application::Impl_::ReloadScene() {
    ASSERT(scene_context_);
    ASSERT(scene_context_->scene);

    name_manager_->Reset();
    event_manager_->Reset();
    panel_manager_->Reset();
    selection_manager_->Reset();
    command_manager_->ResetCommandList();
    tool_manager_->ClearTools();
    Model::ResetColors();

    // Wipe out any events that may be pending in emitters.
    for (auto &emitter: emitters_)
        emitter->FlushPendingEvents();

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

void Application::Impl_::SaveCrashSession(
    const FilePath &path, const std::string &message,
    const std::vector<std::string> &stack) {
    std::vector<std::string> comments;
    comments.reserve(stack.size() + 2);
    comments.push_back(message);
    comments.push_back("---- Stack Trace:");
    Util::AppendVector(stack, comments);

    session_manager_->SaveSessionWithComments(path, comments);

    std::cerr << "*** Saved crash session to " << path.ToString() << "\n";
}

void Application::Impl_::GetTestContext(TestContext &tc) {
    // This should not be called before Init().
    ASSERT(session_manager_);
    ASSERT(scene_context_);
    tc.action_manager    = action_manager_;
    tc.command_manager   = command_manager_;
    tc.panel_manager     = panel_manager_;
    tc.selection_manager = selection_manager_;
    tc.session_manager   = session_manager_;
    tc.settings_manager  = settings_manager_;
    tc.scene_context     = scene_context_;
}

void Application::Impl_::AddEmitter(const IEmitterPtr &emitter) {
    emitters_.push_back(emitter);
}

Vector2i Application::Impl_::GetWindowSize() const {
    return glfw_viewer_->GetWindowSize();
}

void Application::Impl_::InitTypes_() {
    // Register all known concrete types with the Parser::Registry.
    RegisterTypes();
}

bool Application::Impl_::InitViewers_() {
    // Required GLFW viewer.
    glfw_viewer_.reset(new GLFWViewer);
    if (! glfw_viewer_->Init(options_.window_size, options_.fullscreen)) {
        glfw_viewer_.reset();
        return false;
    }
    viewers_.push_back(glfw_viewer_);
    emitters_.push_back(glfw_viewer_);

    // Optional VR viewer.
    vr_context_.reset(new VRContext);
    if (! options_.ignore_vr && vr_context_->InitSystem()) {
        const auto render_func = [&](const SG::Scene &scene, Renderer &renderer,
                                     const Point3f &base_position){
            vr_context_->Render(scene, renderer, base_position);
        };
        const auto emit_func = [&](std::vector<Event> &events,
                                   const Point3f &base_position){
            vr_context_->EmitEvents(events, base_position);
        };
        vr_viewer_.reset(new VRViewer(render_func, emit_func));
        viewers_.push_back(vr_viewer_);
        emitters_.push_back(vr_viewer_);
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
    main_handler_.reset(new MainHandler(IsVREnabled() || options_.enable_vr));

#if ENABLE_DEBUG_FEATURES
    drag_rect_handler_.reset(new DragRectHandler);
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
    event_manager_.reset(new EventManager);
    name_manager_.reset(new NameManager);
    panel_manager_.reset(new PanelManager);
    precision_manager_.reset(new PrecisionManager);
    selection_manager_.reset(new SelectionManager);
    settings_manager_.reset(new SettingsManager);

    // Managers that depend on others.
    board_manager_.reset(new BoardManager(panel_manager_));
    target_manager_.reset(new TargetManager(command_manager_));
    tool_manager_.reset(new ToolManager(*target_manager_));

    settings_manager_->SetChangeFunc(
        [&](const Settings &settings){ SettingsChanged_(settings); });

    // Add all handlers to the EventManager. Order here is extremely
    // important, since Handlers are passed events in this order.
    // LogHandler has to be first so it can log all events.
    event_manager_->AddHandler(log_handler_);
#if ENABLE_DEBUG_FEATURES
    event_manager_->AddHandler(drag_rect_handler_);
#endif
    // ControllerHandler just updates controller position, so it needs all
    // controller events.
    if (IsVREnabled() || options_.enable_vr)
        event_manager_->AddHandler(controller_handler_);
    // InspectorHandler traps most events when active.
    event_manager_->AddHandler(inspector_handler_);
    // Board Handler needs to process keyboard events before others.
    event_manager_->AddHandler(board_handler_);
    event_manager_->AddHandler(shortcut_handler_);
    event_manager_->AddHandler(view_handler_);
    event_manager_->AddHandler(main_handler_);

#if ENABLE_DEBUG_FEATURES
    Debug::SetLogHandler(log_handler_);
#endif

    // The ActionManager requires its own context.
    action_context_.reset(new ActionManager::Context);
    action_context_->scene_context     = scene_context_;
    action_context_->tool_context      = tool_context_;
    action_context_->board_manager     = board_manager_;
    action_context_->clipboard_manager = clipboard_manager_;
    action_context_->command_manager   = command_manager_;
    action_context_->name_manager      = name_manager_;
    action_context_->precision_manager = precision_manager_;
    action_context_->selection_manager = selection_manager_;
    action_context_->settings_manager  = settings_manager_;
    action_context_->target_manager    = target_manager_;
    action_context_->tool_manager      = tool_manager_;
    action_context_->main_handler      = main_handler_;
    action_manager_.reset(new ActionManager(action_context_));
    action_manager_->SetReloadFunc([&]() { ReloadScene(); });

    // Initialize the SessionManager with the previous session path.
    const auto path = settings_manager_->GetSettings().GetLastSessionPath();
    session_manager_.reset(new SessionManager(action_manager_, command_manager_,
                                              selection_manager_, path));

#if ENABLE_DEBUG_FEATURES
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

void Application::Impl_::InitToolContext_() {
    ASSERT(tool_manager_);
    ASSERT(tool_context_);

    tool_context_->board_manager     = board_manager_;
    tool_context_->command_manager   = command_manager_;
    tool_context_->feedback_manager  = feedback_manager_;
    tool_context_->precision_manager = precision_manager_;
    tool_context_->settings_manager  = settings_manager_;
    tool_context_->target_manager    = target_manager_;
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
        panel_context_->command_manager   = command_manager_;
        panel_context_->name_manager      = name_manager_;
        panel_context_->selection_manager = selection_manager_;
        panel_context_->session_manager   = session_manager_;
        panel_context_->settings_manager  = settings_manager_;
        panel_context_->panel_helper      = board_manager_;
        panel_context_->virtual_keyboard  = virtual_keyboard_;
    }
    panel_manager_->FindAllPanels(scene, panel_context_);

    // The TreePanel does not go through the PanelManager, so set it up.
    scene_context_->tree_panel->SetContext(panel_context_);

    // Set up the VirtualKeyboard so that it can make itself visible.
    if (virtual_keyboard_)
        virtual_keyboard_->SetShowHideFunc([&](bool is_shown){
            board_manager_->ShowBoard(scene_context_->key_board, is_shown);
        });

    inspector_handler_->SetInspector(scene_context_->inspector);

#if ENABLE_DEBUG_FEATURES
    drag_rect_handler_->SetSceneContext(scene_context_);
#endif
    board_handler_->AddBoard(scene_context_->key_board);
    board_handler_->AddBoard(scene_context_->app_board);
    board_handler_->AddBoard(scene_context_->tool_board);
    main_handler_->SetSceneContext(scene_context_);
    target_manager_->SetPathToStage(scene_context_->path_to_stage);

    // Inform the viewers and ViewHandler about the cameras in the scene.
    if (! is_testing_) {
        view_handler_->SetCamera(scene_context_->window_camera);
        glfw_viewer_->SetCamera(scene_context_->window_camera);
        if (IsVREnabled())
            vr_viewer_->SetCamera(scene_context_->vr_camera);

        // Store the current camera position in the Tool::Context.
        tool_context_->camera_position =
            scene_context_->window_camera->GetCurrentPosition();
    }

    // Set Nodes in the Controllers.
    auto lc = scene_context_->left_controller;
    auto rc = scene_context_->right_controller;
    ASSERT(lc);
    ASSERT(rc);
    lc->SetHand(Hand::kLeft);
    rc->SetHand(Hand::kRight);
    controller_handler_->SetControllers(lc, rc);
    if (IsVREnabled())
        vr_context_->SetControllers(lc, rc);

    // Enable or disable controllers.
    lc->SetEnabled(IsVREnabled() || options_.enable_vr);
    rc->SetEnabled(IsVREnabled() || options_.enable_vr);

    // Try to use the correct controller models if VR is enabled.
    if (IsVREnabled()) {
        ReplaceControllerModel_(Hand::kLeft);
        ReplaceControllerModel_(Hand::kRight);
    }

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

    // Initialize and add all Grippable objects to the MainHandler.
    AddGrippables_();

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
    wall_board->SetPanelScale(TK::kPanelToWorldScale * 4);  // Far away.
    wall_board->SetPanel(scene_context_->tree_panel);
    board_manager_->ShowBoard(wall_board, true);

    // Set up the other boards for touch mode if in VR or faking it.
    if (IsVREnabled() || options_.set_up_touch) {
        scene_context_->key_board->SetPanel(scene_context_->keyboard_panel);

        const Point3f cam_pos = scene_context_->vr_camera->GetCurrentPosition();
        scene_context_->app_board->SetUpForTouch(cam_pos);
        scene_context_->tool_board->SetUpForTouch(cam_pos);
        scene_context_->key_board->SetUpForTouch(cam_pos);
    }

    // Set up the stage.
    scene_context_->stage->GetClicked().AddObserver(
        this, [&](const ClickInfo &info){
            StartResetStage_(info.is_modified_mode); });

    // Set up the height pole and slider.
    scene_context_->height_pole->GetClicked().AddObserver(
        this, [&](const ClickInfo &info){
            StartResetHeight_(info.is_modified_mode); });
    scene_context_->height_slider->GetClicked().AddObserver(
        this, [&](const ClickInfo &info){
            StartResetHeight_(info.is_modified_mode); });
    scene_context_->height_slider->GetValueChanged().AddObserver(
        this, [&](Widget &w, const float &val){
            scene_context_->gantry->SetHeight(Lerp(val, -10.f, 100.f)); });
    InitTooltip_(*scene_context_->height_pole);
    InitTooltip_(*scene_context_->height_slider);
    // Set up the radial menus.
    InitRadialMenus_();

    // Now that everything has been found, disable searching through the
    // "Definitions" Node.
    SG::FindNodeInScene(scene, "Definitions")->SetFlagEnabled(
        SG::Node::Flag::kSearch, false);

    // Simulate a change in settings to update everything.
    SettingsChanged_(settings_manager_->GetSettings());
}

void Application::Impl_::ReplaceControllerModel_(Hand hand) {
    ASSERT(IsVREnabled());
    Controller::CustomModel model;
    if (vr_context_->LoadControllerModel(hand, model)) {
        auto &controller = hand == Hand::kLeft ?
            *scene_context_->left_controller :
            *scene_context_->right_controller;
        controller.UseCustomModel(model);
    }
}

void Application::Impl_::AddBoards_() {
    ASSERT(scene_context_);
    ASSERT(scene_context_->app_board);
    ASSERT(scene_context_->key_board);
    ASSERT(scene_context_->tool_board);

    tool_context_->board = scene_context_->tool_board;

    // Set a reasonable position for the AppBoard when not in VR.
    scene_context_->app_board->SetPosition(
        Point3f(0, TK::kAppBoardHeight, 0));

    // Install a path filter in the MainHandler that disables interaction with
    // other widgets when the KeyBoard or AppBoard is visible.
    ASSERT(main_handler_);
    auto filter = [&](const SG::NodePath &path){
        auto &kb = scene_context_->key_board;
        auto &ab = scene_context_->app_board;
        return (kb->IsShown() ? Util::Contains(path, kb) :
                ab->IsShown() ? Util::Contains(path, ab) : true);
    };
    main_handler_->SetPathFilter(filter);
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
    Util::AppendVector(scene_context_->precision_control->GetIcons(), icons_);

    // Set up actions and tooltips for all icons.
    for (const auto &icon: icons_) {
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

void Application::Impl_::AddGrippables_() {
    auto add_grippable = [&](const GrippablePtr &grippable){
        // Each Grippable needs to have its path set so it can convert target
        // points to world coordinates.
        SG::Scene &scene = *scene_context_->scene;
        grippable->SetPath(SG::FindNodePathInScene(scene, *grippable));
        main_handler_->AddGrippable(grippable);
    };

    add_grippable(scene_context_->key_board);
    add_grippable(scene_context_->app_board);
    add_grippable(scene_context_->tool_board);
    add_grippable(tool_manager_);
}

void Application::Impl_::InitRadialMenus_() {
    // This is called when a RadialMenu button is clicked.
    auto apply = [&](size_t index, Action action){
        if (action_manager_->CanApplyAction(action))
            action_manager_->ApplyAction(action);
    };
    const auto &lrmenu = scene_context_->left_radial_menu;
    const auto &rrmenu = scene_context_->right_radial_menu;
    lrmenu->GetButtonClicked().AddObserver(this, apply);
    rrmenu->GetButtonClicked().AddObserver(this, apply);

    // Attach the RadialMenu instances to the controllers in case they become
    // visible.
    scene_context_->left_controller->AttachRadialMenu(lrmenu);
    scene_context_->right_controller->AttachRadialMenu(rrmenu);

    // If in VR, turn off the RadialMenu parent in the room, since the menus
    // will be attached to the controllers.
    if (IsVREnabled()) {
        const auto parent =
            SG::FindNodeInScene(*scene_context_->scene, "RadialMenus");
        parent->SetEnabled(false);
    }

    controller_handler_->SetRadialMenus(lrmenu, rrmenu);
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
    const float stage_radius =
        TK::kStageRadiusFraction * std::max(bv_size[0], bv_size[2]);
    scene_context_->stage->SetStageRadius(stage_radius);
}

void Application::Impl_::UpdateIcons_() {
    auto is_enabled = [&](Action action){
        return action_manager_->CanApplyAction(action); };

    for (auto &icon: icons_) {
        ASSERT(icon);

        const bool enabled = is_enabled(icon->GetAction());
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

    // Also update RadialMenu widgets.
    if (scene_context_->left_radial_menu->IsEnabled())
        scene_context_->left_radial_menu->EnableButtons(is_enabled);
    if (scene_context_->right_radial_menu->IsEnabled())
        scene_context_->right_radial_menu->EnableButtons(is_enabled);
}

void Application::Impl_::UpdateGlobalUniforms_() {
    // Get the current world-to-stage matrix.
    const auto wsm =
        SG::CoordConv(scene_context_->path_to_stage).GetRootToObjectMatrix();

    // Get the build volume size. Note that an inactive build volume is
    // indicated by a zero size.
    const auto &bv = *scene_context_->build_volume;
    Vector3f bv_size = bv.IsActive() ? bv.GetSize() : Vector3f::Zero();

    // Update the uniforms.
    scene_context_->root_model->UpdateGlobalUniforms(wsm, bv_size);
}

void Application::Impl_::TryQuit_() {
    // Do nothing if already trying to quit.
    if (run_state_ != RunState_::kRunning)
        return;

    // If there are no changes to the session or the application should not ask
    // the user, just quit.
    if (! session_manager_->CanSaveSession() || ! ask_before_quitting_) {
        run_state_ = RunState_::kQuitting;
        return;
    }

    // Remember that a quit was requested.
    run_state_ = RunState_::kQuitRequested;

    // Open a DialogPanel to verify that the user wants to quit.
    auto func = [&](const std::string &s){
        run_state_ = s == "Yes" ? RunState_::kQuitting : RunState_::kRunning;
    };
    auto dp = board_manager_->GetTypedPanel<DialogPanel>("DialogPanel");
    dp->SetMessage("There are unsaved changes.\nDo you really want to quit?");
    dp->SetChoiceResponse("No", "Yes", true);

    // If the AppBoard is already visible, replace its Panel. Otherwise, just
    // show it with the new Panel.
    const auto &board = scene_context_->app_board;
    if (board->IsShown()) {
        board->PushPanel(dp, func);
    }
    else {
        board->SetPanel(dp, func);
        board_manager_->ShowBoard(board, true);
    }
}

bool Application::Impl_::ProcessEvents_(bool is_modified_mode,
                                        bool force_poll) {
    // Always check for running animations and finished delayed threads.
    const bool is_animating    = animation_manager_->IsAnimating();
    const bool is_any_delaying = Util::IsAnyDelaying();

    // Let the GLFWViewer know whether to poll events or wait for events.  If
    // VR is active, it needs to continuously poll events to track the headset
    // and controllers properly. This means that the GLFWViewer also needs to
    // poll events (rather than wait for them) so as not to block anything. The
    // same is true if the MainHandler is in the middle of handling something
    // (not just waiting for events), if there is an animation running, if
    // something is being delayed, or if something changed in the scene.
    const bool have_to_poll =
        force_poll || IsVREnabled() || is_animating || is_any_delaying ||
        scene_changed_ || ! main_handler_->IsWaiting();
    glfw_viewer_->SetPollEventsFlag(have_to_poll);

    // Collect all incoming events.
    std::vector<Event> events;
    for (auto &emitter: emitters_)
        emitter->EmitEvents(events);

    // Check for an event resulting in quitting. Note that an action that
    // results in quitting invokes the ActionManager's QuitFunc.
    return event_manager_->HandleEvents(events, is_modified_mode,
                                        TK::kMaxEventHandlingTime);
}

void Application::Impl_::ProcessClick_(const ClickInfo &info) {
    // Shorthand for selecting the given Model from the ClickInfo's Hit path.
    auto select_model = [&](const Model &model, bool is_alt){
        const SelPath path(info.hit.path.GetSubPath(model));
        selection_manager_->ChangeModelSelection(path, is_alt);
    };

    KLOG('k', "Click on widget "
         << (info.widget ? info.widget->GetDesc() : "NULL")
         << " is_alt = " << info.is_modified_mode
         << " is_long = " << info.is_long_press);
    if (info.widget) {
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

    // If the intersected object is part of a Tool, process the click as if
    // it were a click on the attached Model.
    else if (ToolPtr tool = info.hit.path.FindNodeUpwards<Tool>()) {
        ASSERT(tool->GetModelAttachedTo());
        select_model(*tool->GetModelAttachedTo(), info.is_modified_mode);
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

void Application::Impl_::StartResetStage_(bool is_modified_mode) {
    // Reset the stage only if alt-clicked.
    if (is_modified_mode) {
        const Vector3f  scale = scene_context_->stage->GetScale();
        const Rotationf rot   = scene_context_->stage->GetRotation();
        animation_manager_->StartAnimation(
            [&, scale, rot](float t){ return ResetStage_(scale, rot, t); });
    }
}

void Application::Impl_::StartResetHeight_(bool is_modified_mode) {
    const float     height = scene_context_->height_slider->GetValue();
    const Rotationf orient = scene_context_->window_camera->GetOrientation();
    const bool      reset_view = is_modified_mode;
    animation_manager_->StartAnimation(
        [&, height, orient, reset_view](float t){
        return ResetHeightAndView_(height, orient, reset_view, t); });
}

bool Application::Impl_::ResetStage_(const Vector3f &start_scale,
                                     const Rotationf &start_rot, float time) {
    // Compute how long the animation should last based on the amount that the
    // scale and rotation have to change.
    const Anglef angle = AbsAngle(RotationAngle(start_rot));
    const float max_scale = start_scale[GetMaxAbsElementIndex(start_scale)];
    const float duration =
        std::max(angle.Degrees() / TK::kMaxStageAngleChangePerSecond,
                 max_scale       / TK::kMaxStageScaleChangePerSecond);

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
    // Compute how long the animation should last based on the amount that the
    // height and view rotation have to change.
    float duration = start_height / TK::kMaxHeightChangePerSecond;
    if (reset_view) {
        const Anglef angle = AbsAngle(RotationAngle(start_view_rot));
        duration = std::max(duration,
                            angle.Degrees() / TK::kMaxViewAngleChangePerSecond);
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
    // Hide Models if the AppBoard is visible.
    return ! scene_context_->app_board->IsShown();
}

Vector3f Application::Impl_::ComputeTooltipTranslation_(
    Widget &widget, const Vector3f &world_size) const {
    // Find a path to the Widget.
    auto path = SG::FindNodePathInScene(*scene_context_->scene, widget);

    // Convert its location to world coordinates.
    const Point3f world_pt = SG::CoordConv(path).ObjectToRoot(Point3f::Zero());

    // Use a plane at a reasonable distance past the image plane of the
    // frustum.
    const auto &frustum = scene_context_->frustum;
    const Point3f  &cam_pos = frustum.position;
    const Vector3f  cam_dir = frustum.orientation * -Vector3f::AxisZ();
    const Plane plane(cam_pos + TK::kTooltipDistance * cam_dir, cam_dir);

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
    const Vector2f half_size = .5f * Vector2f(world_size[0], world_size[1]);
    const float    margin = TK::kTooltipMargin;
    position[0] = Clamp(position[0],
                        ll[0] + margin + half_size[0],
                        ur[0] - margin - half_size[0]);
    position[1] = Clamp(position[1],
                        ll[1] + margin + half_size[1],
                        ur[1] - margin - half_size[1]);

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

bool Application::Init(const Options &options) {
    return impl_->Init(options);
}

void Application::MainLoop() {
    size_t render_count  = 0;

    while (ProcessFrame(render_count, false))
        ++render_count;
}

bool Application::ProcessFrame(size_t render_count, bool force_poll) {
    return impl_->ProcessFrame(render_count, force_poll);
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

void Application::SaveCrashSession(const FilePath &path,
                                   const std::string &message,
                                   const std::vector<std::string> &stack) {
    impl_->SaveCrashSession(path, message, stack);
}

void Application::Shutdown() {
    impl_->Shutdown();
}

void Application::SetTestingFlag() {
    impl_->SetTestingFlag();
}

void Application::SetAskBeforeQuitting(bool ask) {
    impl_->SetAskBeforeQuitting(ask);
}

void Application::GetTestContext(TestContext &tc) {
    impl_->GetTestContext(tc);
}

void Application::AddEmitter(const IEmitterPtr &emitter) {
    impl_->AddEmitter(emitter);
}

Vector2i Application::GetWindowSize() const {
    return impl_->GetWindowSize();
}

Renderer & Application::GetRenderer() {
    return impl_->GetRenderer();
}

void Application::ForceTouchMode(bool is_on) {
    impl_->ForceTouchMode(is_on);
}

void Application::EnableMouseMotionEvents(bool enable) {
    impl_->EnableMouseMotionEvents(enable);
}

void Application::SetControllerRenderOffsets(const Vector3f &l_offset,
                                             const Vector3f &r_offset) {
    impl_->SetControllerRenderOffsets(l_offset, r_offset);
}
