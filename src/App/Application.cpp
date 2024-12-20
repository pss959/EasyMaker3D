//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "App/Application.h"

#include <algorithm>
#include <fstream>
#include <typeinfo>

#include <ion/gfxutils/shadermanager.h>
#include <ion/math/vectorutils.h>

#include "App/ActionProcessor.h"
#include "App/RegisterTypes.h"
#include "App/SceneLoader.h"
#include "App/ToolBox.h"
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
#include "Items/SessionState.h"
#include "Items/Settings.h"
#include "Items/Shelf.h"
#include "Managers/AnimationManager.h"
#include "Managers/BoardManager.h"
#include "Managers/ClipboardManager.h"
#include "Managers/CommandManager.h"
#include "Managers/EventManager.h"
#include "Managers/FeedbackManager.h"
#include "Managers/NameManager.h"
#include "Managers/PanelManager.h"
#include "Managers/SceneContext.h"
#include "Managers/SelectionManager.h"
#include "Managers/SessionManager.h"
#include "Managers/SettingsManager.h"
#include "Managers/TargetManager.h"
#include "Math/Animation.h"
#include "Math/Frustum.h"
#include "Math/Plane.h"
#include "Math/Intersection.h"
#include "Math/Types.h"
#include "Models/Model.h"
#include "Panels/Board.h"
#include "Panels/DialogPanel.h"
#include "Panels/KeyboardPanel.h"
#include "Panels/Panel.h"
#include "Panels/TreePanel.h"
#include "Parser/Exception.h"
#include "Parser/Writer.h"
#include "Place/ClickInfo.h"
#include "Place/PrecisionStore.h"
#include "SG/Change.h"
#include "SG/IonContext.h"
#include "SG/Node.h"
#include "SG/Scene.h"
#include "SG/Search.h"
#include "SG/ShaderProgram.h"
#include "SG/TextNode.h"
#include "SG/VRCamera.h"
#include "SG/WindowCamera.h"
#include "Tools/FindTools.h"
#include "Tools/Tool.h"
#include "Util/Assert.h"
#include "Util/Delay.h"
#include "Util/FilePath.h"
#include "Util/General.h"
#include "Util/KLog.h"
#include "Util/String.h"
#include "Util/Tuning.h"
#include "VR/VRSystem.h"
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
    explicit Impl_(IApplication &app);
    ~Impl_();

    bool Init(const Application::Options &options);

    /// Returns true if VR is enabled (after Init() is called).
    bool IsVREnabled() const { return vr_system_.get(); }

    LogHandler  & GetLogHandler()  const { return *log_handler_;  }
    MainHandler & GetMainHandler() const { return *main_handler_; }

    bool ProcessFrame(size_t render_count, bool force_poll);

    /// Reloads the scene from its path, updating everything necessary.
    void ReloadScene();

    void SaveCrashSession(const FilePath &path, const Str &message,
                          const StrVec &stack);

    void Shutdown() { if (IsVREnabled()) vr_system_->Shutdown(); }

    const Context & GetContext() const { return context_; }
    void SetAskBeforeQuitting(bool ask) { ask_before_quitting_ = ask; }
    void AddEmitter(const IEmitterPtr &emitter);
    Vector2ui GetWindowSize() const;
    IRenderer & GetRenderer() { return *renderer_; }
    void ForceTouchMode(bool is_on) { force_touch_mode_on_ = is_on; }
    void EnableMouseMotionEvents(bool enable) {
        glfw_viewer_->EnableMouseMotionEvents(enable);
    }
    void SetLongPressDuration(float seconds) {
        main_handler_->SetLongPressDuration(seconds);
    }
    void SetTooltipDelay(float seconds);
    bool IsInModifiedMode() const { return glfw_viewer_->IsShiftKeyPressed(); }

  private:
    /// Run states for the main loop.
    enum class RunState_ {
        kRunning,        ///< Running (normal state).
        kQuitRequested,  ///< Processing a request to quit the app.
        kQuitting,       ///< Actually quitting the app.
    };

    IApplication &app_;

    /// Saves Options passed to Init().
    Application::Options options_;

    bool ask_before_quitting_ = true;

    std::unique_ptr<SceneLoader> loader_;

    /// The ToolBox.
    ToolBoxPtr          tool_box_;

    /// \name Systems and Contexts.
    ///@{
    IVRSystemPtr         vr_system_;
    Context              context_;  ///< Contains Managers and SceneContext.
    Tool::ContextPtr     tool_context_;
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

    /// ActionProcessor::Context used to set up the ActionProcessor.
    ActionProcessor::ContextPtr  action_context_;

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

    /// Initializes the session path from the options and the SettingsManager,
    /// returning the path to use, which may be empty.
    FilePath InitSessionPath_();

    ///@}

    /// \name Scene-dependent Initialization

    /// These function initialize some aspect of interaction for the
    /// application. They are called each time the scene is loaded or reloaded.
    ///@{

    /// Wires up all interaction that depends on items in the scene.
    void ConnectSceneInteraction_();

    /// Wires up the height slider and pole.
    void ConnectHeightSlider_();

    /// If possible, replaces the controller model for the given hand with one
    /// supplied by the VRContext.
    void ReplaceControllerModel_(Hand hand);

    /// Adds the Board instances for 2D-ish UI.
    void AddBoards_();

    /// Adds templates for all Tools to the ToolBox.
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

    /// Loads settings if possible from the user's settings file.
    void LoadSettings_();

    /// Displays a DialogPanel informing the user about an error loading
    /// settings.
    void ShowSettingsError_(const Str &error);

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

    /// Moves the gantry based on the given height slider value.
    void MoveGantry_(float height_slider_val);

    /// Initiates animation to reset the stage.
    void StartResetStage_(bool is_modified_mode);

    /// Initiates animation to reset the height slider.
    void StartResetHeight_(bool is_modified_mode);

    /// Animation callback function to reset the stage.
    bool ResetStage_(float start_scale, const Anglef &start_angle, float time);

    /// Animation callback function to reset the height and optionally the
    /// view direction.
    bool ResetHeightAndView_(float start_height,
                             const Rotationf &start_view_rot,
                             bool reset_view, float time);

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

// Shorthand. E.g., MGR_(Session) => context_.session_manager_.
#define MGR_(NAME) context_.NAME ## _manager
#define AP_        context_.action_processor
#define SC_        context_.scene_context
#define PREC_      context_.precision_store

Application::Impl_::Impl_(IApplication &app) : app_(app),
                                               loader_(new SceneLoader) {
}

Application::Impl_::~Impl_() {
    // Do nothing if Init() failed.
    if (! SC_ || ! SC_->scene)
        return;

    MGR_(event)->ClearHandlers();
    viewers_.clear();
    emitters_.clear();

    // Instances must be destroyed in a particular order.
#if ENABLE_DEBUG_FEATURES
    Debug::ShutDown();
#endif
    view_handler_.reset();
    SC_.reset();
    renderer_.reset();
    vr_system_.reset();
    glfw_viewer_.reset();
}

bool Application::Impl_::Init(const Application::Options &options) {
    options_ = options;

    // Note that order here is extremely important!

    InitTypes_();

    // Make sure the scene loads properly and has all of the necessary items
    // (in the SceneContext) before doing anything else.
    SC_.reset(new SceneContext);
    const FilePath scene_path = FilePath::GetResourcePath(
        "scenes", "workshop" + TK::kDataFileExtension);
    SG::ScenePtr scene = loader_->LoadScene(scene_path);
    if (! scene) {
#if RELEASE_BUILD
        throw Parser::Exception("Error loading scene from resources");
#endif
        return false;
    }
    SC_->FillFromScene(scene, true);
#if ENABLE_DEBUG_FEATURES
    Debug::SetSceneContext(SC_);
#endif

    if (Util::app_type != Util::AppType::kUnitTest) {
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
            vr_system_->InitRendering(*renderer_);
        virtual_keyboard_.reset(new VirtualKeyboard);
    }

    // This needs to exist for the ActionProcessor.
    tool_context_.reset(new Tool::Context);

    // Set up the tooltip function for use in all Widgets and Models.
    tooltip_func_ = [&](Widget &widget, const Str &text, bool show){
        const Str key = Util::ToString(&widget);
        if (show) {
            auto tf = MGR_(feedback)->ActivateWithKey<TooltipFeedback>(key);
            tf->SetText(text);
            auto tpath = SG::FindNodePathInScene(*SC_->scene, *tf);
            // Object coordinates for the tooltip object are the same as world
            // coordinates except for the scale applied to the tooltip object,
            // so just scale the size.
            tf->SetTranslation(
                ComputeTooltipTranslation_(widget,
                                           tf->GetScale() * tf->GetTextSize()));
            tf->SetRotation(ComputeTooltipRotation_());
        }
        else {
            MGR_(feedback)->DeactivateWithKey<TooltipFeedback>(key);
        }
    };

    InitHandlers_();
    InitManagers_();
    InitExecutors_();
    InitToolContext_();
    InitInteraction_();

    // Install things...
    main_handler_->SetPrecisionStore(PREC_);
    shortcut_handler_->SetActionAgent(AP_);

    ConnectSceneInteraction_();

    // Try to read shortcuts from a "shortcuts.txt" file in the current
    // directory. Quit on error after showing an error message.
    const FilePath path("shortcuts.txt");
    Str errors;
    if (! shortcut_handler_->AddCustomShortcutsFromFile(path, errors)) {
        auto dp = MGR_(board)->GetTypedPanel<DialogPanel>("DialogPanel");
        dp->SetMessage("Error in custom shortcut file:\n" + errors);
        dp->SetSingleResponse("OK");
        auto board = SC_->app_board;
        board->SetPanel(dp, [&](const Str &){ TryQuit_(); });
        MGR_(board)->ShowBoard(board, true);
        return true;  // Continue so that the user sees the message.
    }

    // If loading settings failed, show an error message and quit. Otherwise,
    // show the initial Panel if requested.
    if (options.show_session_panel) {
        const Str &settings_error = MGR_(settings)->GetLoadError();
        if (! settings_error.empty())
            ShowSettingsError_(settings_error);
        else
            ShowInitialPanel_();
    }

    // Tell the ActionProcessor how to quit.
    AP_->SetQuitFunc([&]{ TryQuit_(); });

    return true;
}

bool Application::Impl_::ProcessFrame(size_t render_count, bool force_poll) {
    ASSERT(run_state_ != RunState_::kQuitting);

    const bool is_modified_mode = app_.IsInModifiedMode();

    KLogger::SetRenderCount(render_count++);
    renderer_->BeginFrame();

    // Update global uniforms in the RootModel.
    UpdateGlobalUniforms_();

    // Update everything that needs it.
    main_handler_->SetTouchable(MGR_(board)->GetCurrentBoard());
    main_handler_->ProcessUpdate(is_modified_mode);
    tool_context_->is_modified_mode = is_modified_mode;

    AP_->ProcessUpdate();

    // Process any animations. Do this after updating the MainHandler
    // because a click timeout may start an animation.
    MGR_(animation)->ProcessUpdate();

    // Enable or disable all icon widgets and update tooltips.
    UpdateIcons_();

    // If an application panel is visible, hide the scene and disable
    // application shortcuts (main app only).
    const bool is_app_panel_shown = SC_->app_board->IsShown();
    SC_->work_hider->SetEnabled(! is_app_panel_shown);
    if (Util::app_type == Util::AppType::kMainApp)
        shortcut_handler_->SetAppShortcutsEnabled(! is_app_panel_shown);

    // Put controllers in touch mode if the AppBoard, KeyBoard, or
    // ToolBoard is active.
    const bool in_touch_mode =
        force_touch_mode_on_ ||
        SC_->app_board->IsShown() ||
        SC_->key_board->IsShown() ||
        SC_->tool_board->IsShown();
    SC_->left_controller->SetTouchMode(in_touch_mode);
    SC_->right_controller->SetTouchMode(in_touch_mode);

    // Emit and process Events. This returns false if the application
    // should quit because the window was closed.
    if (! ProcessEvents_(is_modified_mode, force_poll))
        TryQuit_();

    // Update the TreePanel.
    SC_->tree_panel->SetSessionString(MGR_(session)->GetSessionString());

    // Update the current tool if there is one and it is attached to a Model.
    // Do this after processing events so the world is up to date.
    auto tool = tool_box_->GetCurrentTool();
    if (tool && tool->GetModelAttachedTo())
        tool->Update();

    // Clear this flag before rendering. Rendering might cause some changes
    // to occur, and those may need to be detected.
    scene_changed_ = false;

    // Render to all viewers.
    for (auto &viewer: viewers_) {
        KLOG('R', "Render to " << Util::Demangle(typeid(*viewer).name()));
        viewer->Render(*SC_->scene, *renderer_);
    }

    renderer_->EndFrame();

    return run_state_ != RunState_::kQuitting;
}

void Application::Impl_::ReloadScene() {
    ASSERT(SC_);
    ASSERT(SC_->scene);

    MGR_(name)->Reset();
    MGR_(event)->Reset();
    MGR_(panel)->Reset();
    MGR_(selection)->Reset();
    MGR_(command)->ResetCommandList();
    tool_box_->ClearTools();
    Model::ResetColors();

    // Wipe out any events that may be pending in emitters.
    for (auto &emitter: emitters_)
        emitter->FlushPendingEvents();

    try {
        SG::ScenePtr scene =
            loader_->LoadScene(SC_->scene->GetPath());
        SC_->FillFromScene(scene, true);
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

void Application::Impl_::SaveCrashSession(const FilePath &path,
                                          const Str &message,
                                          const StrVec &stack) {
    StrVec comments;
    comments.reserve(stack.size() + 2);
    comments.push_back(message);
    comments.push_back("---- Stack Trace:");
    Util::AppendVector(stack, comments);

    // Save the session to the crash file if possible. If the problem happens
    // before a successful init, just save the stack.
    if (MGR_(session)) {
        MGR_(session)->SaveSessionWithComments(path, comments);
    }
    else {
        std::ofstream out(path.ToNativeString());
        if (out) {
            Parser::Writer writer(out);
            for (const auto &comment: comments)
                writer.WriteComment(comment);
        }
    }
    std::cerr << "*** Saved crash session to " << path.ToString() << "\n";
}

void Application::Impl_::AddEmitter(const IEmitterPtr &emitter) {
    emitters_.push_back(emitter);
}

Vector2ui Application::Impl_::GetWindowSize() const {
    return glfw_viewer_->GetWindowSize();
}

void Application::Impl_::SetTooltipDelay(float seconds) {
    // Setting this should trigger the SettingsChanged_() callback that updates
    // the TooltipFeedback delay.
    auto &mgr = *MGR_(settings);
    const auto &settings = mgr.GetSettings();
    auto new_settings = Settings::CreateCopy(settings);
    new_settings->SetTooltipDelay(seconds);
    mgr.SetSettings(*new_settings);
}

void Application::Impl_::InitTypes_() {
    // Register all known concrete types with the Parser::Registry.
    RegisterTypes();
}

bool Application::Impl_::InitViewers_() {
    // Required GLFW viewer.
    auto error_func = [](const Str &error){
        std::cerr << "*** " << error << "\n";
    };
    glfw_viewer_.reset(new GLFWViewer(error_func));
    if (! glfw_viewer_->Init(options_.window_size, options_.maximize,
                             options_.offscreen)) {
        glfw_viewer_.reset();
        return false;
    }
    viewers_.push_back(glfw_viewer_);
    emitters_.push_back(glfw_viewer_);

    // Optional VR viewer.
    vr_system_.reset(new VRSystem);
    if (options_.connect_vr && vr_system_->Startup()) {
        vr_viewer_.reset(new VRViewer(vr_system_));
        viewers_.push_back(vr_viewer_);
        emitters_.push_back(vr_viewer_);
    }
    else {
        vr_system_.reset();
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
    main_handler_.reset(new MainHandler(IsVREnabled() || options_.display_vr));

#if ENABLE_DEBUG_FEATURES
    drag_rect_handler_.reset(new DragRectHandler);
    Debug::SetLogHandler(log_handler_);
#endif
}

void Application::Impl_::InitManagers_() {
    ASSERT(main_handler_);
    ASSERT(tool_context_);

    MGR_(animation).reset(new AnimationManager);
    MGR_(clipboard).reset(new ClipboardManager);
    MGR_(feedback).reset(new FeedbackManager);
    MGR_(command).reset(new CommandManager);
    MGR_(event).reset(new EventManager);
    MGR_(name).reset(new NameManager);
    MGR_(panel).reset(new PanelManager);
    PREC_.reset(new PrecisionStore);
    MGR_(selection).reset(new SelectionManager);
    MGR_(settings).reset(new SettingsManager);

    // Managers that depend on others.
    MGR_(board).reset(new BoardManager(MGR_(panel)));
    MGR_(target).reset(new TargetManager(MGR_(command)));
    tool_box_ = Parser::Registry::CreateObject<ToolBox>();

    tool_box_->SetTargetManager(*MGR_(target));

    // Load settings. This has to be done before initializing other Managers.
    LoadSettings_();
    MGR_(settings)->SetChangeFunc(
        [&](const Settings &settings){ SettingsChanged_(settings); });

    // Add all handlers to the EventManager. Order here is extremely
    // important, since Handlers are passed events in this order.
    // LogHandler has to be first so it can log all events.
    MGR_(event)->AppendHandler(log_handler_);
#if ENABLE_DEBUG_FEATURES
    MGR_(event)->AppendHandler(drag_rect_handler_);
#endif
    // ControllerHandler just updates controller position, so it needs all
    // controller events.
    if (IsVREnabled() || options_.display_vr)
        MGR_(event)->AppendHandler(controller_handler_);
    // InspectorHandler traps most events when active.
    MGR_(event)->AppendHandler(inspector_handler_);
    // Board Handler needs to process keyboard events before others.
    MGR_(event)->AppendHandler(board_handler_);
    MGR_(event)->AppendHandler(shortcut_handler_);
    MGR_(event)->AppendHandler(view_handler_);
    MGR_(event)->AppendHandler(main_handler_);

#if ENABLE_DEBUG_FEATURES
    Debug::SetLogHandler(log_handler_);
#endif

    // The ActionProcessor requires its own context.
    action_context_.reset(new ActionProcessor::Context);
    action_context_->scene_context     = SC_;
    action_context_->tool_box          = tool_box_;
    action_context_->board_manager     = MGR_(board);
    action_context_->clipboard_manager = MGR_(clipboard);
    action_context_->command_manager   = MGR_(command);
    action_context_->name_manager      = MGR_(name);
    action_context_->precision_store   = PREC_;
    action_context_->selection_manager = MGR_(selection);
    action_context_->settings_manager  = MGR_(settings);
    action_context_->target_manager    = MGR_(target);
    action_context_->main_handler      = main_handler_;
    AP_.reset(new ActionProcessor(action_context_));
    AP_->SetReloadFunc([&]() { ReloadScene(); });

    const auto session_path = InitSessionPath_();
    MGR_(session).reset(new SessionManager(AP_, MGR_(command),
                                           MGR_(selection), session_path));

#if ENABLE_DEBUG_FEATURES
    Debug::SetCommandList(MGR_(command)->GetCommandList());
#endif
}

void Application::Impl_::InitExecutors_() {
    ASSERT(MGR_(command));

    executors_ = InitExecutors();

    exec_context_.reset(new Executor::Context);
    exec_context_->animation_manager = MGR_(animation);
    exec_context_->clipboard_manager = MGR_(clipboard);
    exec_context_->command_manager   = MGR_(command);
    exec_context_->name_manager      = MGR_(name);
    exec_context_->selection_manager = MGR_(selection);
    exec_context_->settings_manager  = MGR_(settings);
    exec_context_->target_manager    = MGR_(target);
    exec_context_->tooltip_func      = tooltip_func_;
    for (auto &exec: executors_) {
        exec->SetContext(exec_context_);
        auto func = [exec](Command &cmd,
                           Command::Op op){ exec->Execute(cmd, op); };
        MGR_(command)->RegisterFunction(exec->GetCommandTypeName(), func);
    }
}

void Application::Impl_::InitToolContext_() {
    ASSERT(tool_box_);
    ASSERT(tool_context_);

    tool_context_->board_manager     = MGR_(board);
    tool_context_->command_manager   = MGR_(command);
    tool_context_->feedback_manager  = MGR_(feedback);
    tool_context_->precision_store   = PREC_;
    tool_context_->settings_manager  = MGR_(settings);
    tool_context_->target_manager    = MGR_(target);
}

void Application::Impl_::InitInteraction_() {
    ASSERT(main_handler_);

    main_handler_->GetClicked().AddObserver(
        this, [&](const ClickInfo &info){ ProcessClick_(info); });

    // Detect selection changes to update the ToolBox.
    MGR_(selection)->GetSelectionChanged().AddObserver(
        this, [&](const Selection &sel, SelectionManager::Operation op){
            SelectionChanged_(sel, op); });
}

FilePath Application::Impl_::InitSessionPath_() {
    // Initialize the SessionManager with the session file specified on the
    // command line or the previous session if there was none.
    const auto &settings = MGR_(settings)->GetSettings();

    FilePath session_path = options_.session_file_name;
    if (session_path) {
        if (! session_path.IsAbsolute())
            session_path =
                FilePath::Join(settings.GetSessionDirectory(), session_path);
        // Save the path.
        auto new_settings = Settings::CreateCopy(settings);
        new_settings->SetLastSessionPath(session_path);
        MGR_(settings)->SetSettings(*new_settings);
    }
    else {
        session_path = settings.GetLastSessionPath();
    }
    return session_path;
}

void Application::Impl_::ConnectSceneInteraction_() {
    ASSERT(SC_);
    ASSERT(SC_->scene);

    // Use the frustum from the GLFWViewer.
    if (glfw_viewer_)
        SC_->frustum = glfw_viewer_->GetFrustum();
    else
        SC_->frustum.reset(new Frustum);

    MGR_(board)->SetFrustum(SC_->frustum);

    auto &scene = *SC_->scene;

    // Tell the ActionProcessor::Context about the new Scene.
    action_context_->scene_context = SC_;

    // Tell the SelectionManager and Executor::Context about the new RootModel.
    MGR_(selection)->SetRootModel(SC_->root_model);
    exec_context_->root_model = SC_->root_model;

    // Set up the Panel::Context and let the PanelManager find the new Panel
    // instances.
    if (! panel_context_) {
        panel_context_.reset(new Panel::Context);
        panel_context_->action_agent     = AP_;
        panel_context_->name_agent       = MGR_(name);
        panel_context_->selection_agent  = MGR_(selection);
        panel_context_->session_agent    = MGR_(session);
        panel_context_->settings_agent   = MGR_(settings);
        panel_context_->board_agent      = MGR_(board);
        panel_context_->virtual_keyboard = virtual_keyboard_;
    }
    MGR_(panel)->FindAllPanels(scene, panel_context_);

    // The TreePanel does not go through the PanelManager, so set it up.
    SC_->tree_panel->SetContext(panel_context_);

    // Set up the VirtualKeyboard so that it can make itself visible.
    if (virtual_keyboard_)
        virtual_keyboard_->SetShowHideFunc([&](bool is_shown){
            MGR_(board)->ShowBoard(SC_->key_board, is_shown);
        });

    inspector_handler_->SetInspector(SC_->inspector);

#if ENABLE_DEBUG_FEATURES
    drag_rect_handler_->SetFrustum(SC_->frustum);
    drag_rect_handler_->SetRect(SC_->debug_rect);
#endif
    board_handler_->AddBoard(SC_->key_board);
    board_handler_->AddBoard(SC_->app_board);
    board_handler_->AddBoard(SC_->tool_board);
    board_handler_->AddBoard(SC_->wall_board);

    MainHandler::Context mc;
    mc.scene            = SC_->scene;
    mc.frustum          = SC_->frustum;
    mc.path_to_stage    = SC_->path_to_stage;
    mc.left_controller  = SC_->left_controller;
    mc.right_controller = SC_->right_controller;
    mc.debug_sphere     = SC_->debug_sphere;
    main_handler_->SetContext(mc);

    MGR_(target)->SetPathToStage(SC_->path_to_stage);

    // Inform the viewers and ViewHandler about the cameras in the scene.
    if (Util::app_type != Util::AppType::kUnitTest) {
        view_handler_->SetCamera(SC_->window_camera);
        glfw_viewer_->SetCamera(SC_->window_camera);
        if (IsVREnabled())
            vr_system_->SetCamera(SC_->vr_camera);

        // Store the current camera position in the Tool::Context.
        tool_context_->camera_position =
            SC_->window_camera->GetCurrentPosition();
    }

    // Set Nodes in the Controllers.
    auto lc = SC_->left_controller;
    auto rc = SC_->right_controller;
    ASSERT(lc);
    ASSERT(rc);
    lc->SetHand(Hand::kLeft);
    rc->SetHand(Hand::kRight);
    controller_handler_->SetControllers(lc, rc);
    if (IsVREnabled())
        vr_system_->SetControllers(lc, rc);

    // Enable or disable controllers.
    lc->SetEnabled(IsVREnabled() || options_.display_vr);
    rc->SetEnabled(IsVREnabled() || options_.display_vr);

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
    MGR_(target)->InitTargets(
        SG::FindTypedNodeInScene<PointTargetWidget>(scene, "PointTargetWidget"),
        SG::FindTypedNodeInScene<EdgeTargetWidget>(scene,  "EdgeTargetWidget"));

    // Hook up the exit sign.
    auto exit_sign =
        SG::FindTypedNodeInScene<PushButtonWidget>(scene, "ExitSign");
    exit_sign->GetClicked().AddObserver(
        this, [this](const ClickInfo &){
            AP_->ApplyAction(Action::kQuit);
        });
    InitTooltip_(*exit_sign);

    // Set up the TreePanel.
    auto wall_board = SG::FindTypedNodeInScene<Board>(scene, "WallBoard");
    wall_board->SetDistanceScale(4);  // Far away.
    wall_board->SetPanel(SC_->tree_panel);
    MGR_(board)->ShowBoard(wall_board, true);

    // Set up the other boards for touch mode if in VR or faking it.
    if (IsVREnabled() || options_.set_up_touch) {
        SC_->key_board->SetPanel(SC_->keyboard_panel);

        const Point3f cam_pos = SC_->vr_camera->GetCurrentPosition();
        SC_->app_board->SetUpForTouch(cam_pos);
        SC_->tool_board->SetUpForTouch(cam_pos);
        SC_->key_board->SetUpForTouch(cam_pos);
    }

    // Set up the stage.
    auto reset_stage = [&](const ClickInfo &info){
        if (app_.IsAnimationEnabled())
            StartResetStage_(info.is_modified_mode);
        else
            SC_->stage->SetScaleAndRotation(1, Anglef());
    };
    SC_->stage->GetClicked().AddObserver(this, reset_stage);

    // Set up the height pole and slider.
    ConnectHeightSlider_();

    InitTooltip_(*SC_->height_pole);
    InitTooltip_(*SC_->height_slider);
    // Set up the radial menus.
    InitRadialMenus_();

    // Now that everything has been found, disable searching through the
    // "Definitions" Node.
    SG::FindNodeInScene(scene, "Definitions")->SetFlagEnabled(
        SG::Node::Flag::kSearch, false);

    // Simulate a change in settings to update everything.
    SettingsChanged_(MGR_(settings)->GetSettings());
}

void Application::Impl_::ConnectHeightSlider_() {
    auto reset_slider = [&](const ClickInfo &info){
        if (app_.IsAnimationEnabled()) {
            StartResetHeight_(info.is_modified_mode);
        }
        else {
            // Immediate reset.
            auto &hs = *SC_->height_slider;
            hs.SetValue(hs.GetInitialValue());
            if (info.is_modified_mode)
                SC_->window_camera->SetOrientation(Rotationf::Identity());
        }
    };

    // Clicking the height pole or slider may reset the slider.
    SC_->height_pole->GetClicked().AddObserver(this, reset_slider);
    SC_->height_slider->GetClicked().AddObserver(this, reset_slider);

    // Dragging the height slider changes the gantry height.
    SC_->height_slider->GetValueChanged().AddObserver(
        this, [&](Widget &w, const float &val){ MoveGantry_(val); });

    // Set the initial value T for the slider to put the gantry at Y=0.
    //    Lerp(T, min, max) = 0
    //       min + T * (max - min) = 0
    //       T = -min / (max - min)
    SC_->height_slider->SetInitialValue(
        -TK::kMinGantryHeight / (TK::kMaxGantryHeight - TK::kMinGantryHeight));
}

void Application::Impl_::ReplaceControllerModel_(Hand hand) {
    ASSERT(IsVREnabled());
    Controller::CustomModel model;
    if (vr_system_->LoadControllerModel(hand, model)) {
        auto &controller = hand == Hand::kLeft ?
            *SC_->left_controller :
            *SC_->right_controller;
        controller.UseCustomModel(model);
    }
}

void Application::Impl_::AddBoards_() {
    ASSERT(SC_);
    ASSERT(SC_->app_board);
    ASSERT(SC_->key_board);
    ASSERT(SC_->tool_board);

    tool_context_->board = SC_->tool_board;

    // Set a reasonable position for the AppBoard when not in VR.
    SC_->app_board->SetPosition(
        Point3f(0, TK::kAppBoardHeight, 0));

    // Install a path filter in the MainHandler that disables interaction with
    // other widgets when the KeyBoard or AppBoard is visible.
    ASSERT(main_handler_);
    auto filter = [&](const SG::NodePath &path){
        auto &kb = SC_->key_board;
        auto &ab = SC_->app_board;
        return (kb->IsShown() ? Util::Contains(path, kb) :
                ab->IsShown() ? Util::Contains(path, ab) : true);
    };
    main_handler_->SetPathFilter(filter);
}

void Application::Impl_::AddTools_() {
    ASSERT(tool_box_);
    ASSERT(SC_);
    ASSERT(SC_->scene);

    tool_context_->root_model = SC_->root_model;

    SG::Scene &scene = *SC_->scene;
    auto path_to_parent = SG::FindNodePathInScene(scene, "ToolParent");
    tool_box_->ClearTools();
    tool_box_->SetParentNode(path_to_parent.back());
    tool_context_->path_to_parent_node = path_to_parent;

    // Find the parent node containing all Tools in the scene and use it to get
    // all Tool instances.
    const std::vector<ToolPtr> tools = FindTools(scene);
    for (auto &tool: tools)
        tool->SetContext(tool_context_);
    tool_box_->AddTools(tools);
    tool_box_->SetDefaultGeneralTool("TranslationTool");
}

void Application::Impl_::AddFeedback_() {
    ASSERT(SC_);
    ASSERT(SC_->scene);
    ASSERT(MGR_(feedback));

    SG::Scene &scene = *SC_->scene;

    const SG::NodePtr world_fb_parent =
        SG::FindNodeInScene(scene, "WorldFeedbackParent");
    const SG::NodePtr stage_fb_parent =
        SG::FindNodeInScene(scene, "StageFeedbackParent");
    MGR_(feedback)->Reset();
    MGR_(feedback)->SetParentNodes(world_fb_parent, stage_fb_parent);
    MGR_(feedback)->SetSceneBoundsFunc([this](){
        return SC_->root_model->GetBounds(); });
    MGR_(feedback)->SetPathToStage(SC_->path_to_stage);

    for (auto &fb: FindFeedback(*scene.GetRootNode()))
        MGR_(feedback)->AddOriginal<Feedback>(fb);
}

void Application::Impl_::AddIcons_() {
    ASSERT(AP_);
    ASSERT(SC_);
    ASSERT(SC_->scene);

    icons_.clear();

    Point3f cam_pos;
    if (Util::app_type == Util::AppType::kUnitTest) {
        cam_pos = Point3f(0, 0, 100);
    }
    else {
        ASSERT(glfw_viewer_);
        cam_pos = glfw_viewer_->GetFrustum()->position;
    }

    // Set up the icons on the shelves.
    SG::Scene &scene = *SC_->scene;
    const SG::NodePtr shelves_root = SG::FindNodeInScene(scene, "Shelves");
    const auto is_shelf = [](const SG::Node &node){
        return dynamic_cast<const Shelf *>(&node) != nullptr;
    };
    for (const auto &child: FindNodes(shelves_root, is_shelf)) {
        const ShelfPtr shelf = std::dynamic_pointer_cast<Shelf>(child);
        ASSERT(shelf);
        shelf->LayOutIcons(cam_pos);
        Util::AppendVector(shelf->GetIcons(), icons_);
    }

    // PrecisionControl is a special case.
    Util::AppendVector(SC_->precision_control->GetIcons(), icons_);

    // Set up actions and tooltips for all icons.
    for (const auto &icon: icons_) {
        icon->GetClicked().AddObserver(
            this, [&](const ClickInfo &){
                AP_->ApplyAction(icon->GetAction());});
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
        SG::Scene &scene = *SC_->scene;
        grippable->SetPath(SG::FindNodePathInScene(scene, *grippable));
        main_handler_->AddGrippable(grippable);
    };

    add_grippable(SC_->key_board);
    add_grippable(SC_->app_board);
    add_grippable(SC_->tool_board);
    add_grippable(tool_box_);
}

void Application::Impl_::InitRadialMenus_() {
    // This is called when a RadialMenu button is clicked.
    auto apply = [&](size_t index, Action action){
        if (AP_->CanApplyAction(action))
            AP_->ApplyAction(action);
    };
    const auto &lrmenu = SC_->left_radial_menu;
    const auto &rrmenu = SC_->right_radial_menu;
    lrmenu->GetButtonClicked().AddObserver(this, apply);
    rrmenu->GetButtonClicked().AddObserver(this, apply);

    // Attach the RadialMenu instances to the controllers in case they become
    // visible.
    SC_->left_controller->AttachRadialMenu(lrmenu);
    SC_->right_controller->AttachRadialMenu(rrmenu);

    // If in VR, turn off the RadialMenu parent in the room, since the menus
    // will be attached to the controllers.
    if (IsVREnabled()) {
        const auto parent =
            SG::FindNodeInScene(*SC_->scene, "RadialMenus");
        parent->SetEnabled(false);
    }

    controller_handler_->SetRadialMenus(lrmenu, rrmenu);
}

void Application::Impl_::ShowInitialPanel_() {
    // Show the SessionPanel.
    AP_->ApplyAction(Action::kOpenSessionPanel);
}

void Application::Impl_::InitTooltip_(Widget &widget) {
    ASSERT(tooltip_func_);
    widget.SetTooltipFunc(tooltip_func_);
}

void Application::Impl_::LoadSettings_() {
    const auto settings_path = FilePath::Join(
        FilePath::GetSettingsDirPath(TK::kApplicationName),
        "settings" + TK::kDataFileExtension);
    MGR_(settings)->SetPath(settings_path, true);
}

void Application::Impl_::ShowSettingsError_(const Str &error) {
    // If there is an error, display a message in a DialogPanel and quit.
    auto dp = MGR_(board)->GetTypedPanel<DialogPanel>("DialogPanel");
    dp->SetMessage("Error reading settings from " +
                   MGR_(settings)->GetPath().ToString() + ":\n" + error);
    dp->SetSingleResponse("OK");
    SC_->app_board->SetPanel(dp, [&](const Str &){
        run_state_ = RunState_::kQuitting; });
    MGR_(board)->ShowBoard(SC_->app_board, true);
}

void Application::Impl_::SelectionChanged_(const Selection &sel,
                                           SelectionManager::Operation op) {
    // Don't update tools while loading a session.
    if (! MGR_(command)->IsValidating()) {
        switch (op) {
            using enum SelectionManager::Operation;
          case kSelection:
            tool_box_->AttachToSelection(sel);
            // If this is the first time the primary selected Model was selected,
            // try to use a specialized tool for it.
            if (sel.HasAny() &&
                sel.GetPrimary().GetModel()->GetSelectionCount() == 1U)
                tool_box_->UseSpecializedTool(sel);
            break;
          case kReselection:
            tool_box_->ReattachTools();
            break;
          case kDeselection:
            tool_box_->DetachTools(sel);
            break;
          case kUpdate:
            // Nothing to do in this case.
            break;
        }
    }
    SC_->tree_panel->ModelsChanged();
}

void Application::Impl_::SettingsChanged_(const Settings &settings) {
    TooltipFeedback::SetDelay(settings.GetTooltipDelay());

    SC_->left_radial_menu->UpdateFromInfo(
        settings.GetLeftRadialMenuInfo());
    SC_->right_radial_menu->UpdateFromInfo(
        settings.GetRightRadialMenuInfo());

    /// Update the build volume size.
    const auto &bv_size = settings.GetBuildVolumeSize();
    SC_->build_volume->SetSize(bv_size);

    /// Update the stage radius based on the build volume size.
    const float stage_radius =
        TK::kStageRadiusFraction * std::max(bv_size[0], bv_size[2]);
    SC_->stage->SetStageRadius(stage_radius);
}

void Application::Impl_::UpdateIcons_() {
    auto is_enabled = [&](Action action){
        return AP_->CanApplyAction(action); };

    for (auto &icon: icons_) {
        ASSERT(icon);

        const bool enabled = is_enabled(icon->GetAction());
        icon->SetInteractionEnabled(enabled);

        const Action action = icon->GetAction();
        icon->SetTooltipText(AP_->GetActionTooltip(action));

        if (enabled && icon->IsToggle())
            icon->SetToggleState(AP_->GetToggleState(action));
    }

    // Special case for the ToggleSpecializedToolIcon.
    const auto &sel = MGR_(selection)->GetSelection();
    const auto tool = tool_box_->GetSpecializedToolForSelection(sel);
    const Str tool_name = tool ? tool->GetTypeName() : "Null";
    toggle_specialized_tool_icon_->SetIndexByName(tool_name + "Icon");
    toggle_specialized_tool_icon_->SetToggleState(
        tool_box_->IsUsingSpecializedTool());

    // Also update RadialMenu widgets.
    if (SC_->left_radial_menu->IsEnabled())
        SC_->left_radial_menu->EnableButtons(is_enabled);
    if (SC_->right_radial_menu->IsEnabled())
        SC_->right_radial_menu->EnableButtons(is_enabled);
}

void Application::Impl_::UpdateGlobalUniforms_() {
    // Get the current world-to-stage matrix.
    const auto wsm =
        SG::CoordConv(SC_->path_to_stage).GetRootToObjectMatrix();

    // Get the build volume size. Note that an inactive build volume is
    // indicated by a zero size.
    const auto &bv = *SC_->build_volume;
    Vector3f bv_size = bv.IsActive() ? bv.GetSize() : Vector3f::Zero();

    // Update the uniforms.
    SC_->root_model->UpdateGlobalUniforms(wsm, bv_size);
}

void Application::Impl_::TryQuit_() {
    // Do nothing if already trying to quit.
    if (run_state_ != RunState_::kRunning)
        return;

    // If there are no changes to the session or the application should not ask
    // the user, just quit.
    if (! MGR_(session)->CanSaveSession() || ! ask_before_quitting_) {
        run_state_ = RunState_::kQuitting;
        return;
    }

    // Remember that a quit was requested.
    run_state_ = RunState_::kQuitRequested;

    // Open a DialogPanel to verify that the user wants to quit.
    auto func = [&](const Str &s){
        run_state_ = s == "Yes" ? RunState_::kQuitting : RunState_::kRunning;
    };
    auto dp = MGR_(board)->GetTypedPanel<DialogPanel>("DialogPanel");
    dp->SetMessage("There are unsaved changes.\nDo you really want to quit?");
    dp->SetChoiceResponse("No", "Yes", true);

    // If the AppBoard is already visible, replace its Panel. Otherwise, just
    // show it with the new Panel.
    const auto &board = SC_->app_board;
    if (board->IsShown()) {
        board->PushPanel(dp, func);
    }
    else {
        board->SetPanel(dp, func);
        MGR_(board)->ShowBoard(board, true);
    }
}

bool Application::Impl_::ProcessEvents_(bool is_modified_mode,
                                        bool force_poll) {
    // Always check for running animations and finished delayed threads.
    const bool is_animating    = MGR_(animation)->IsAnimating();
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
    // results in quitting invokes the ActionProcessor's QuitFunc.
    return MGR_(event)->HandleEvents(events, is_modified_mode,
                                     TK::kMaxEventHandlingTime);
}

void Application::Impl_::ProcessClick_(const ClickInfo &info) {
    if (info.widget) {
        KLOG('k', "Click on widget " << info.widget->GetDesc()
             << " is_mod = " << info.is_modified_mode
             << " is_long = " << info.is_long_press);
        if (info.widget->IsInteractionEnabled()) {
            info.widget->Click(info);
            // If long pressing on a Model, select and inspect it.
            if (info.is_long_press) {
                auto model = dynamic_cast<Model *>(info.widget);
                if (model) {
                    const SelPath path(info.hit.path.GetSubPath(*model));
                    MGR_(selection)->ChangeModelSelection(path, false);
                    // Make sure that the action is enabled.
                    AP_->ProcessUpdate();
                    AP_->ApplyAction(Action::kToggleInspector);
                }
            }
        }
    }

    // If the intersected object is part of a Tool, process the click as if
    // it were a click on the attached (primary) Model. This has an effect only
    // for a modified-click.
    else if (ToolPtr tool = info.hit.path.FindNodeUpwards<Tool>()) {
        KLOG('k', "Click on tool " << tool->GetDesc()
             << " is_mod = " << info.is_modified_mode);
        if (info.is_modified_mode) {
            const auto &sel = MGR_(selection)->GetSelection();
            ASSERT(sel.HasAny());
            MGR_(selection)->ChangeModelSelection(sel.GetPrimary(), true);
        }
    }

    // If the intersected object is part of a Board, ignore the click.
    else if (info.hit.path.FindNodeUpwards<Board>()) {
        // Do nothing.
        KLOG('k', "Click on Board");
    }

    // Otherwise, the click was on a noninteractive object, so deselect.
    else {
        KLOG('k', "Click on noninteractive object");
        MGR_(selection)->DeselectAll();
    }
}

void Application::Impl_::MoveGantry_(float height_slider_val) {
    SC_->gantry->SetHeight(Lerp(height_slider_val,
                                TK::kMinGantryHeight, TK::kMaxGantryHeight));
}

void Application::Impl_::StartResetStage_(bool is_modified_mode) {
    // Reset the stage only if modified-clicked.
    if (is_modified_mode) {
        const auto scale = SC_->stage->GetScale()[0];
        const auto angle = RotationAngle(SC_->stage->GetRotation());
        MGR_(animation)->StartAnimation(
            [&, scale, angle](float t){ return ResetStage_(scale, angle, t); });
    }
}

void Application::Impl_::StartResetHeight_(bool is_modified_mode) {
    const float     height = SC_->height_slider->GetValue();
    const Rotationf orient = SC_->window_camera->GetOrientation();
    const bool      reset_view = is_modified_mode;
    MGR_(animation)->StartAnimation(
        [&, height, orient, reset_view](float t){
        return ResetHeightAndView_(height, orient, reset_view, t); });
}

bool Application::Impl_::ResetStage_(float start_scale,
                                     const Anglef &start_angle, float time) {
    // Compute how long the animation should last based on the amount that the
    // scale and rotation have to change.
    const float abs_degrees = std::abs(start_angle.Degrees());
    const float duration =
        std::max(abs_degrees / TK::kMaxStageAngleChangePerSecond,
                 start_scale / TK::kMaxStageScaleChangePerSecond);

    // Interpolate and update the stage's scale and rotation.
    const float t = std::min(1.f, time / duration);
    StageWidget &stage = *SC_->stage;
    const auto scale = Lerp(t, start_scale, 1.f);
    const auto angle = Lerp(t, start_angle, Anglef());
    stage.SetScaleAndRotation(scale, angle);

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
    Slider1DWidget &slider = *SC_->height_slider;
    slider.SetValue(Lerp(t, start_height, slider.GetInitialValue()));
    if (reset_view) {
        SC_->window_camera->SetOrientation(
            Rotationf::Slerp(start_view_rot, Rotationf::Identity(), t));
    }
    // Keep going until finished.
    return t < 1.f;
}

Vector3f Application::Impl_::ComputeTooltipTranslation_(
    Widget &widget, const Vector3f &world_size) const {
    // Find a path to the Widget.
    auto path = SG::FindNodePathInScene(*SC_->scene, widget);

    // Convert its location to world coordinates.
    const Point3f world_pt = SG::CoordConv(path).ObjectToRoot(Point3f::Zero());

    // Use a plane at a reasonable distance past the image plane of the
    // frustum.
    const auto &frustum = *SC_->frustum;
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
    return SC_->frustum->orientation;
}

#undef MGR_
#undef AP_
#undef SC_
#undef PREC_

// ----------------------------------------------------------------------------
// Application functions.
// ----------------------------------------------------------------------------

Application::Application() : impl_(new Impl_(*this)) {
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

MainHandler & Application::GetMainHandler() const {
    return impl_->GetMainHandler();
}

bool Application::IsVREnabled() const {
    return impl_->IsVREnabled();
}

void Application::SaveCrashSession(const FilePath &path, const Str &message,
                                   const StrVec &stack) {
    impl_->SaveCrashSession(path, message, stack);
}

void Application::Shutdown() {
    impl_->Shutdown();
}

void Application::SetAskBeforeQuitting(bool ask) {
    impl_->SetAskBeforeQuitting(ask);
}

const Application::Context & Application::GetContext() const {
    return impl_->GetContext();
}

void Application::AddEmitter(const IEmitterPtr &emitter) {
    impl_->AddEmitter(emitter);
}

Vector2ui Application::GetWindowSize() const {
    return impl_->GetWindowSize();
}

IRenderer & Application::GetRenderer() {
    return impl_->GetRenderer();
}

void Application::ForceTouchMode(bool is_on) {
    impl_->ForceTouchMode(is_on);
}

void Application::EnableMouseMotionEvents(bool enable) {
    impl_->EnableMouseMotionEvents(enable);
}

void Application::SetLongPressDuration(float seconds) {
    impl_->SetLongPressDuration(seconds);
}

void Application::SetTooltipDelay(float seconds) {
    impl_->SetTooltipDelay(seconds);
}

bool Application::IsInModifiedMode() const {
    return impl_->IsInModifiedMode();
}
