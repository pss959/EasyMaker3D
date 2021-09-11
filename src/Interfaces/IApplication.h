#pragma once

#include <memory>
#include <vector>

#include "Math/Types.h"
#include "SG/Context.h"
#include "SG/Scene.h"
#include "Interfaces/IInterfaceBase.h"
#include "Util/FilePath.h"

class IHandler;
class Renderer;
class Viewer;

//! Abstract Interface class defining an Application. It is a factory for the
//! other interfaces used in the application.
//!
//! \ingroup Interfaces
class IApplication : public IInterfaceBase {
  public:
    //! This struct contains all of the interfaces an application must support.
    struct Context {
        //! SG::Context used to set up Ion objects in the scene graph.
        SG::ContextPtr             sg_context;

        //! Scene representing everything to be rendered.
        SG::ScenePtr               scene;

        //! Renderer used to render to all viewers.
        std::shared_ptr<Renderer>  renderer;

        //! List of Viewer instances that can view a rendered scene. Note that
        //! these are raw pointers; the derived class is required to guarantee
        //! lifetimes.
        std::vector<Viewer *>      viewers;

        //! List of IHandler instances that handle events. Note that these are
        //! raw pointers; the derived class is required to guarantee lifetimes.
        std::vector<IHandler *>    handlers;
    };

    //! Initializes the application and creates a filled-in Context. The
    //! default window size is passed in for viewers that need it.
    virtual void Init(const Vector2i &window_size) = 0;

    //! Returns the Context created in Init();
    virtual Context & GetContext() = 0;

    //! Reloads the scene from its path and updates the application to show it.
    virtual void ReloadScene() = 0;
};
