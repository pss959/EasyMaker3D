#pragma once

#include <stack>

#include <ion/gfx/shaderinputregistry.h>
#include <ion/gfxutils/shadermanager.h>
#include <ion/text/fontmanager.h>

#include "Assert.h"
#include "Parser/Object.h"

namespace SG {

class Tracker;

//! Object is an abstract base class for all SG classes. It supports naming.
class Object : public Parser::Object {
  public:
    //! This struct is passed to SetUpIon so that derived classes have context
    //! for creating and tracking Ion objects.
    struct IonContext {
        //! Tracker instance used to determine if Ion objects have already been
        //! loaded.
        Tracker &tracker;

        //! ShaderManager for creating shaders.
        ion::gfxutils::ShaderManagerPtr shader_manager;

        //! FontManager used for text.
        ion::text::FontManagerPtr font_manager;

        //! Stack of Ion ShaderInputRegistry instances. The topmost registry
        //! should be used for creating uniforms and shaders. This may be
        //! modified as the SG graph is traversed.
        std::stack<ion::gfx::ShaderInputRegistryPtr> registry_stack;

        //! The constructor is passed the Tracker and ShaderManager to use. It
        //! pushes the global registry on the stack.
        IonContext(Tracker &tracker_in,
                   const ion::gfxutils::ShaderManagerPtr &shader_manager_in,
                   const ion::text::FontManagerPtr &font_manager_in) :
            tracker(tracker_in),
            shader_manager(shader_manager_in),
            font_manager(font_manager_in) {
            ASSERT(shader_manager);
            ASSERT(font_manager);
            registry_stack.push(
                ion::gfx::ShaderInputRegistry::GetGlobalRegistry());
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
