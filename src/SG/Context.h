#pragma once

#include <memory>
#include <stack>
#include <string>

#include <ion/gfx/shaderinputregistry.h>
#include <ion/gfxutils/shadermanager.h>
#include <ion/text/fontmanager.h>

#include "SG/PassType.h"
#include "SG/Tracker.h"

namespace SG {

//! This struct is stored in each instance so that the SetUpIon() function
//! in derived classes has context for creating and tracking Ion objects.
struct Context {
    //! Tracker instance used to determine if Ion objects have already been
    //! loaded.
    TrackerPtr tracker;

    //! ShaderManager for creating shaders.
    ion::gfxutils::ShaderManagerPtr shader_manager;

    //! FontManager used for text.
    ion::text::FontManagerPtr font_manager;

    //! PassType of the current render pass being set up.
    PassType pass_type;

    //! Stack of Ion ShaderInputRegistry instances. The topmost registry
    //! should be used for creating uniforms and shaders. This may be
    //! modified as the SG graph is traversed.
    std::stack<ion::gfx::ShaderInputRegistryPtr> registry_stack;

    //! The constructor is passed the Tracker and ShaderManager to use. It
    //! pushes the global registry on the stack.
    Context(const TrackerPtr &tracker_in,
            const ion::gfxutils::ShaderManagerPtr &shader_manager_in,
            const ion::text::FontManagerPtr &font_manager_in) :
        tracker(tracker_in),
        shader_manager(shader_manager_in),
        font_manager(font_manager_in) {
        ASSERT(shader_manager);
        ASSERT(font_manager);
        registry_stack.push(ion::gfx::ShaderInputRegistry::GetGlobalRegistry());
    }
};

typedef std::shared_ptr<Context> ContextPtr;

}  // namespace SG
