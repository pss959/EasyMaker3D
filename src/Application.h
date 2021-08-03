#pragma once

#include "Interfaces/IApplication.h"

class GLFWViewer;
class OpenXRVR;

//! Application is an implementation of the IApplication interface. It is
//! basically a factory that produces instances of implementations of all
//! required interfaces.
class Application : public IApplication {
  public:
    Application();
    ~Application();

    virtual Context & Init(const ion::math::Vector2i &window_size) override;

  private:
    //! Derived Context that has storage for necessary classes.
    struct Context_ : public Context {
        //! GLFWViewer instance used for window display.
        std::shared_ptr<GLFWViewer> glfw_viewer_;
        //! OpenXRVR instance used for VR interaction.
        std::shared_ptr<OpenXRVR>   openxrvr_;

        Context_();
        ~Context_();

        //! Initializes the Context_.
        void Init(const ion::math::Vector2i &window_size);
    };

    Context_ context_;
};
