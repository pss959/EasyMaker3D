#pragma once

#include "Hand.h"
#include "Interfaces/IApplication.h"

class Controller;
class GLFWViewer;
class LogHandler;
class OpenXRVR;
class ViewHandler;

//! Application is an implementation of the IApplication interface. It is
//! basically a factory that produces instances of implementations of all
//! required interfaces.
class Application : public IApplication {
  public:
    Application();
    virtual ~Application();

    virtual const char * GetClassName() const override {
        return "Application";
    }
    virtual Context & Init(const ion::math::Vector2i &window_size) override;

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

    // XXXX Temporary workaround for OpenXR xrDestroyInstance() problem.
    bool ShouldKillApp() const { return IsVREnabled(); }

  private:
    //! Derived Context that has storage for necessary classes.
    struct Context_ : public Context {
        //! Managed GLFWViewer instance used for window display.
        std::unique_ptr<GLFWViewer>  glfw_viewer_;

        //! Managed OpenXRVR instance used for VR interaction.
        std::unique_ptr<OpenXRVR>    openxrvr_;

        //! Managed ViewHandler instance used for view interaction.
        std::unique_ptr<ViewHandler> view_handler_;

        //! Managed LogHandler that can be enabled to help with debugging or
        //! testing.
        std::unique_ptr<LogHandler>  log_handler_;

        //! Left hand controller.
        std::unique_ptr<Controller>  l_controller_;
        //! Right hand controller.
        std::unique_ptr<Controller>  r_controller_;

        Context_();
        ~Context_();

        //! Initializes the Context_.
        void Init(const ion::math::Vector2i &window_size);
    };

    Context_ context_;
};
