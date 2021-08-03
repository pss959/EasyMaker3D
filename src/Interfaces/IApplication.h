#pragma once

#include <memory>
#include <vector>

#include <ion/math/vector.h>

#include "Interfaces/IInterfaceBase.h"

class IEmitter;
class IHandler;
class IRenderer;
class IScene;
class IVR;
class IViewer;

//! Abstract Interface class defining an Application. It is a factory for all
//! of the other interfaces used in the application.
//! \ingroup Interfaces
class IApplication : public IInterfaceBase {
  public:
    //! This struct contains all of the interfaces an application must support.
    struct Context {
        //! Renderer used to render to all viewers.
        std::shared_ptr<IRenderer> renderer;

        //! Scene representing everything to be rendered.
        std::shared_ptr<IScene>    scene;

        //! XXXX GET RID OF THIS
        std::shared_ptr<IVR>       vr;

        //! List of IViewer instances that can view a rendered scene. Note that
        //! these are raw pointers; the derived class is required to guarantee
        //! lifetimes.
        std::vector<IViewer *> viewers;

        //! List of IEmitter instances that can produce events. Note that these
        //! are raw pointers; the derived class is required to guarantee
        //! lifetimes.
        std::vector<IEmitter *> emitters;

        //! List of IHandler instances that handle events. Note that these are
        //! raw pointers; the derived class is required to guarantee lifetimes.
        std::vector<IHandler *> handlers;
    };

    //! Initializes the application and returns a filled-in Context. The
    //! default window size is passed in for viewers that need it.
    virtual Context & Init(const ion::math::Vector2i &window_size) = 0;
};
