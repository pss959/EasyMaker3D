#pragma once

#include "Enums/Hand.h"
#include "Interfaces/IApplication.h"
#include "Math/Types.h"
#include "SG/IonSetup.h"
#include "SceneContext.h"

class AnimationManager;
class Controller;
class GLFWViewer;
class LogHandler;
class MainHandler;
class ShortcutHandler;
class VRContext;
class VRViewer;
class ViewHandler;
struct ClickInfo;

/// Application is an implementation of the IApplication interface. It is
/// basically a factory that produces instances of implementations of all
/// required interfaces.
class Application : public IApplication {
  public:
    Application();
    virtual ~Application();

    // ------------------------------------------------------------------------
    // IApplication interface.
    // ------------------------------------------------------------------------

    virtual const char * GetClassName() const override {
        return "Application";
    }
    virtual void      Init(const Vector2i &window_size) override;
    virtual Context & GetContext() override;
    virtual void      ReloadScene() override;

    // ------------------------------------------------------------------------
    // Other public interface.
    // ------------------------------------------------------------------------

    /// Enters the main loop.
    void MainLoop();

    /// Returns the LogHandler so it can be enabled or disabled.
    LogHandler & GetLogHandler() const { return *context_.log_handler_; }

    /// Returns the Controller for the given Hand.
    Controller & GetController(Hand hand) const {
        return hand == Hand::kLeft ?
            *context_.l_controller_ : *context_.r_controller_;
    }

    /// Returns true if virtual reality is active, meaning that a headset is
    /// connected.
    bool IsVREnabled() const { return context_.IsVREnabled(); }

    /// Temporary workaround for OpenXR xrDestroyInstance() problem.
    /// TODO: Remove this if OpenXR gets fixed.
    bool ShouldKillApp() const { return IsVREnabled(); }

  private:
    /// Derived Context that has storage for necessary classes.
    struct Context_ : public Context {
        /// Managed SG::IonSetup.
        std::unique_ptr<SG::IonSetup>     ion_setup_;

        /// Managed AnimationManager.
        std::unique_ptr<AnimationManager> animation_manager_;

        /// Managed GLFWViewer instance used for window display.
        std::unique_ptr<GLFWViewer>      glfw_viewer_;

        /// Managed VRContext instance used for VR setup.
        std::unique_ptr<VRContext>       vr_context_;

        /// Managed VRViewer instance used for VR viewing.
        std::unique_ptr<VRViewer>        vr_viewer_;

        /// Managed ViewHandler instance used for view interaction.
        std::unique_ptr<ViewHandler>     view_handler_;

        /// Managed MainHandler that handles most of the interaction.
        std::unique_ptr<MainHandler>     main_handler_;

        /// Managed LogHandler that can be enabled to help with debugging or
        /// testing.
        std::unique_ptr<LogHandler>      log_handler_;

        /// Managed ShortcutHandler.
        std::unique_ptr<ShortcutHandler> shortcut_handler_;

        /// Left hand controller.
        std::unique_ptr<Controller>      l_controller_;
        /// Right hand controller.
        std::unique_ptr<Controller>      r_controller_;

        /// Managed SceneContext.
        std::shared_ptr<SceneContext>    scene_context_;

        Context_();
        ~Context_();

        /// Initializes the Context_.
        void Init(const Vector2i &window_size, IApplication &app);

        /// Returns true if VR is enabled (after Init() is called).
        bool IsVREnabled() const { return vr_context_.get(); }

        /// Reloads the scene from its path, updating viewers.
        void ReloadScene();

        /// Updates the SceneContext after a load or reload.
        void UpdateSceneContext_();

        /// Wires up all in-scene interaction.
        void ConnectSceneInteraction_();

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

    Context_ context_;
};
