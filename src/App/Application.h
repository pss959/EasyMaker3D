//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include "Math/Types.h"
#include "App/IApplication.h"
#include "Util/Memory.h"

class Controller;
class FilePath;
class IRenderer;
class LogHandler;
class MainHandler;

DECL_SHARED_PTR(ActionProcessor);
DECL_SHARED_PTR(AnimationManager);
DECL_SHARED_PTR(BoardManager);
DECL_SHARED_PTR(ClipboardManager);
DECL_SHARED_PTR(CommandManager);
DECL_SHARED_PTR(EventManager);
DECL_SHARED_PTR(FeedbackManager);
DECL_SHARED_PTR(IEmitter);
DECL_SHARED_PTR(NameManager);
DECL_SHARED_PTR(PanelManager);
DECL_SHARED_PTR(PrecisionStore);
DECL_SHARED_PTR(SceneContext);
DECL_SHARED_PTR(SelectionManager);
DECL_SHARED_PTR(SessionManager);
DECL_SHARED_PTR(SettingsManager);
DECL_SHARED_PTR(TargetManager);

/// The Application class manages the entire application.
///
/// \ingroup App
class Application : public IApplication {
  public:
    /// Application options.
    struct Options {
        Vector2ui window_size;
        bool      do_ion_remote      = false;
        bool      maximize           = false;
        bool      display_vr         = false;
        bool      connect_vr         = false;
        bool      set_up_touch       = false;
        bool      show_session_panel = true;
        bool      offscreen          = false;
        Str       session_file_name;
    };

    /// Application context with all managers and the SceneContext.
    struct Context {
        /// \name Managers.
        ///@{
        ActionProcessorPtr  action_processor;
        AnimationManagerPtr animation_manager;
        BoardManagerPtr     board_manager;
        ClipboardManagerPtr clipboard_manager;
        CommandManagerPtr   command_manager;
        EventManagerPtr     event_manager;
        FeedbackManagerPtr  feedback_manager;
        NameManagerPtr      name_manager;
        PanelManagerPtr     panel_manager;
        PrecisionStorePtr   precision_store;
        SelectionManagerPtr selection_manager;
        SessionManagerPtr   session_manager;
        SettingsManagerPtr  settings_manager;
        TargetManagerPtr    target_manager;
        ///@}

        /// SceneContext.
        SceneContextPtr     scene_context;
    };

    Application();
    ~Application();

    /// Initializes the application.
    bool Init(const Options &options);

    /// Enters the main loop.
    void MainLoop();

    /// Processes a single frame during the main loop. Returns false if the
    /// main loop should exit. If the force_poll flag is true, this forces the
    /// event-checking code to poll instead of waiting for the next event.
    virtual bool ProcessFrame(size_t render_count, bool force_poll);

    /// Reloads the scene.
    void ReloadScene();

    /// Returns the LogHandler so it can be enabled or disabled.
    LogHandler & GetLogHandler() const;

    /// Returns the MainHandler so it can be modified.
    MainHandler & GetMainHandler() const;

    /// Returns true if virtual reality is active, meaning that a headset is
    /// connected.
    bool IsVREnabled() const;

    /// Tells the SessionManager to save the current session to the given file
    /// with the given crash message and stack trace.
    virtual void SaveCrashSession(const FilePath &path, const Str &message,
                                  const StrVec &stack) override;

    /// Shuts down (especially VR which does not like to be crashed out of.
    virtual void Shutdown() override;

  protected:
    /// Sets a flag indicating whether the application should ask about saving
    /// before quitting a session. The default is true.
    void SetAskBeforeQuitting(bool ask);

    /// Returns the current Context.
    const Context & GetContext() const;

    /// Adds an event IEmitter to the list of emitters.
    void AddEmitter(const IEmitterPtr &emitter);

    /// Returns the current size of the application window, taking the maximize
    /// option into account.
    Vector2ui GetWindowSize() const;

    /// Returns the IRenderer.
    IRenderer & GetRenderer();

    /// Allows derived classes to force touch mode to be on.
    void ForceTouchMode(bool is_on);

    /// Allows derived classes to temporarily ignore mouse motion events
    /// produced by GLFW.
    void EnableMouseMotionEvents(bool enable);

    /// Allows derived classes to change the minimum duration in seconds for a
    /// button press to be considered a long press. The default is
    /// TK::kLongPressTime.
    void SetLongPressDuration(float seconds);

    /// Allows derived classes to change the tooltip delay. This updates the
    /// current settings as well as TooltipFeedback so that the setting is
    /// persistent.
    void SetTooltipDelay(float seconds);

    /// Defines this to ask the GLFWViewer if the shift key is pressed.
    virtual bool IsInModifiedMode() const override;

    /// Defines this to always return true.
    virtual bool IsAnimationEnabled() const override { return true; }

  private:
    class  Loader_;
    class  Impl_;
    std::unique_ptr<Impl_> impl_;
};
