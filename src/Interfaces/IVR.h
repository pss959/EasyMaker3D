#pragma once

#include <exception>
#include <string>

class IRenderer;
class IScene;

//! Abstract Interface class for VR systems.
//! \ingroup Interfaces
class IVR {
  public:
    // Exception thrown when any function fails.
    class VRException : public std::exception {
      public:
        VRException(const std::string &msg) : msg_(msg) {}
        const char * what() const throw() override { return msg_.c_str(); }
      private:
        std::string msg_;
    };

    //! Initializes the VR system, returning false if it is not available.
    virtual bool Init() = 0;

    //! Initializes for rendering using the given IRenderer.
    virtual void InitRendering(IRenderer &renderer) = 0;

    //! Renders the given Scene using the given IRenderer.
    virtual void Render(IScene &scene, IRenderer &renderer) = 0;

    //! Polls and handles events from the VR system. Returns false if the
    //! application should exit.
    virtual bool PollEvents() = 0;
};
