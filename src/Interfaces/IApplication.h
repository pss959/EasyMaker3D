#pragma once

#include <memory>
#include <vector>

#include <ion/gfxutils/shadermanager.h>
#include <ion/math/vector.h>

#include "SG/Scene.h"
#include "Interfaces/IInterfaceBase.h"
#include "Util/FilePath.h"

class IEmitter;
class IHandler;
class IRenderer;
class IResourceManager;
class IViewer;

//! Abstract Interface class defining an Application. It is a factory for the
//! other interfaces used in the application.
//!
//! \ingroup Interfaces
class IApplication : public IInterfaceBase {
  public:
    //! This struct contains all of the interfaces an application must support.
    struct Context {
        //! Ion shader manager used to create all shaders in the app.
        ion::gfxutils::ShaderManagerPtr shader_manager;

        //! Renderer used to render to all viewers.
        std::shared_ptr<IRenderer>      renderer;

        //! Scene representing everything to be rendered.
        SG::ScenePtr                    scene;

        //! List of IViewer instances that can view a rendered scene. Note that
        //! these are raw pointers; the derived class is required to guarantee
        //! lifetimes.
        std::vector<IViewer *>          viewers;

        //! List of IEmitter instances that can produce events. Note that these
        //! are raw pointers; the derived class is required to guarantee
        //! lifetimes.
        std::vector<IEmitter *>         emitters;

        //! List of IHandler instances that handle events. Note that these are
        //! raw pointers; the derived class is required to guarantee lifetimes.
        std::vector<IHandler *>         handlers;
    };

    //! Initializes the application and creates a filled-in Context. The
    //! default window size is passed in for viewers that need it.
    virtual void Init(const ion::math::Vector2i &window_size) = 0;

    //! Returns the Context created in Init();
    virtual Context & GetContext() = 0;

    //! Reloads the scene from its path and updates the application to show it.
    virtual void ReloadScene() = 0;
};
