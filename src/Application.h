#pragma once

#include "Interfaces/IApplication.h"

class GLFWViewer;
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

    // XXXX Temporary workaround for OpenXR xrDestroyInstance() problem.
    bool ShouldKillApp() const { return context_.openxrvr_.get(); }

  private:
    //! Derived Context that has storage for necessary classes.
    struct Context_ : public Context {
        //! Managed GLFWViewer instance used for window display.
        std::unique_ptr<GLFWViewer>  glfw_viewer_;

        //! Managed OpenXRVR instance used for VR interaction.
        std::unique_ptr<OpenXRVR>    openxrvr_;

        //! Managed ViewHandler instance used for view interaction.
        std::unique_ptr<ViewHandler> view_handler_;

        Context_();
        ~Context_();

        //! Initializes the Context_.
        void Init(const ion::math::Vector2i &window_size);
    };

    Context_ context_;
};
