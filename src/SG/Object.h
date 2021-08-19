#pragma once

#include <ion/gfx/shaderinputregistry.h>
#include <ion/gfxutils/shadermanager.h>

#include "NParser/Object.h"

namespace SG {

class Tracker;

//! Object is an abstract base class for all SG classes. It supports naming.
class Object : public NParser::Object {
  public:
    //! This struct is passed to SetUpIon so that derived classes have context
    //! for creating and tracking Ion objects.
    struct IonContext {
        //! Tracker instance used to determine if Ion objects have already been
        //! loaded.
        Tracker &tracker;

        //! ShaderManager for creating shaders.
        ion::gfxutils::ShaderManager &shader_manager;

        //! Current Ion ShaderInputRegistry to use for creating uniforms and
        //! shaders. This may be modified as the SG graph is traversed.
        ion::gfx::ShaderInputRegistryPtr current_registry;

        //! The constructor is passed the Tracker and ShaderManager to use. It
        //! sets the current_registry to the global registry.
        IonContext(Tracker &tracker_in,
                   ion::gfxutils::ShaderManager &shader_manager_in) :
            tracker(tracker_in),
            shader_manager(shader_manager_in),
            current_registry(
                ion::gfx::ShaderInputRegistry::GetGlobalRegistry()) {
        }
    };

    virtual ~Object() {}

    //! This is called after a scene is read to set up Ion types in all SG
    //! objects. Each derived class should implement this to recurse on all
    //! sub-objects and set up itself. The base class defines this to do
    //! nothing.
    virtual void SetUpIon(IonContext &context) {}

  protected:
    //! The constructor is protected to make this abstract.
    Object() {}
};

}  // namespace SG
