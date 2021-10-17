#pragma once

#include <memory>
#include <vector>

#include <ion/gfxutils/shadermanager.h>
#include <ion/text/fontmanager.h>

#include "Enums/Action.h"
#include "Math/Types.h"
#include "SG/Tracker.h"
#include "SG/Typedefs.h"
#include "Interfaces/IInterfaceBase.h"
#include "Util/FilePath.h"

class Handler;
class Renderer;
class Viewer;

/// Abstract Interface class defining an Application. It is a factory for the
/// other interfaces used in the application.
///
/// \ingroup Interfaces
class IApplication : public IInterfaceBase {
  public:
    /// This struct contains all of the interfaces an application must support.
    struct Context {
        /// SG::Tracker used to manage resources.
        SG::TrackerPtr                  tracker;

        // ShaderManager for creating shaders.
        ion::gfxutils::ShaderManagerPtr shader_manager;

        // FontManager for text.
        ion::text::FontManagerPtr       font_manager;

        /// Scene representing everything to be rendered.
        SG::ScenePtr                    scene;

        /// Renderer used to render to all viewers.
        std::unique_ptr<Renderer>       renderer;

        /// List of Viewer instances that can view a rendered scene. Note that
        /// these are raw pointers; the derived class is required to guarantee
        /// lifetimes.
        std::vector<Viewer *>           viewers;

        /// List of Handler instances that handle events. Note that these are
        /// raw pointers; the derived class is required to guarantee lifetimes.
        std::vector<Handler *>          handlers;
    };

    /// Initializes the application and creates a filled-in Context. The
    /// default window size is passed in for viewers that need it.
    virtual void Init(const Vector2i &window_size) = 0;

    /// Returns the Context created in Init();
    virtual Context & GetContext() = 0;

    /// Reloads the scene from its path and updates the application to show it.
    virtual void ReloadScene() = 0;
};
