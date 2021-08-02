#pragma once

class IRenderer;
class IScene;
class IVR;

//! Abstract Interface class defining an Application. It is a factory for all
//! of the other interfaces used in the application.
//! \ingroup Interfaces
class IApplication {
  public:
    //! Returns an IRenderer to use for rendering, creating it first if
    //! necessary.
    virtual IRenderer & GetRenderer() = 0;

    //! Returns an IScene representing the scene, creating it first if
    //! necessary
    virtual IScene & GetScene() = 0;

    //! Returns an IVR representing a VR system, creating it first if necessary
    virtual IVR & GetVR() = 0;
};
