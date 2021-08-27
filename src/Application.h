#pragma once

#include "Hand.h"
#include "Interfaces/IApplication.h"
#include "SG/Typedefs.h"

class Controller;
class GLFWViewer;
class LogHandler;
class OpenXRVR;
class ShortcutHandler;
class ViewHandler;

//! Application is an implementation of the IApplication interface. It is
//! basically a factory that produces instances of implementations of all
//! required interfaces.
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
    virtual void      Init(const ion::math::Vector2i &window_size) override;
    virtual Context & GetContext() override;
    virtual void      ReloadScene() override;

    // ------------------------------------------------------------------------
    // Other public interface.
    // ------------------------------------------------------------------------

    //! Enters the main loop.
    void MainLoop();

    //! Returns the LogHandler so it can be enabled or disabled.
    LogHandler & GetLogHandler() const { return *context_.log_handler_; }

    //! Returns the Controller for the given Hand.
    Controller & GetController(Hand hand) const {
        return hand == Hand::kLeft ?
            *context_.l_controller_ : *context_.r_controller_;
    }

    //! Returns true if virtual reality is active, meaning that a headset is
    //! connected.
    bool IsVREnabled() const { return context_.openxrvr_.get(); }

    //! Temporary workaround for OpenXR xrDestroyInstance() problem.
    //! TODO: Remove this if OpenXR gets fixed.
    bool ShouldKillApp() const { return IsVREnabled(); }

  private:
    //! Derived Context that has storage for necessary classes.
    struct Context_ : public Context {
        //! SG::Tracker used for tracking scene graph resources.
        std::unique_ptr<SG::Tracker>     tracker_;

        //! Managed GLFWViewer instance used for window display.
        std::unique_ptr<GLFWViewer>      glfw_viewer_;

        //! Managed OpenXRVR instance used for VR interaction.
        std::unique_ptr<OpenXRVR>        openxrvr_;

        //! Managed ViewHandler instance used for view interaction.
        std::unique_ptr<ViewHandler>     view_handler_;

        //! Managed LogHandler that can be enabled to help with debugging or
        //! testing.
        std::unique_ptr<LogHandler>      log_handler_;

        //! Managed ShortcutHandler.
        std::unique_ptr<ShortcutHandler> shortcut_handler_;

        //! Left hand controller.
        std::unique_ptr<Controller>      l_controller_;
        //! Right hand controller.
        std::unique_ptr<Controller>      r_controller_;

        //! Debug TextNode.
        SG::TextNodePtr                  debug_text_;

        Context_();
        ~Context_();

        //! Initializes the Context_.
        void Init(const ion::math::Vector2i &window_size, IApplication &app);

        //! Reloads the scene from its path, updating viewers.
        void ReloadScene();

        //! Updates the View in all viewers when the scene changes.
        void UpdateViews_();
    };

    Context_ context_;
};
