#pragma once

#include <ion/gfxutils/shadermanager.h>

#include "Memory.h"

DECL_SHARED_PTR(Renderer);

struct FBTarget;
struct Frustum;
namespace SG { class Scene; }

/// A Renderer is used to render a scene in a viewer.
class Renderer {
  public:
    /// The constructor passed an Ion ShaderManager and a flag indicating
    /// whether to set up Ion remote debugging.
    Renderer(const ion::gfxutils::ShaderManagerPtr &shader_manager,
             bool use_ion_remote);

    ~Renderer();

    /// Creates a framebuffer that can be used as a render target, returning
    /// its index.
    int CreateFramebuffer();

    /// Resets everything (e.g., after a reload). The new Scene is supplied for
    /// setting up remote debugging.
    void Reset(const SG::Scene &scene);

    /// Returns the current frame count. The count is reset to 0 when Reset()
    /// is called.
    uint64_t GetFrameCount() const;

    /// Renders the given Scene using the given Frustum. If fb_target is not
    /// null, it is used instead of the default target.
    void RenderScene(const SG::Scene &scene, const Frustum &frustum,
                     const FBTarget *fb_target = nullptr);

  private:
    class Impl_; // This class does most of the work.

    std::unique_ptr<Impl_> impl_;  ///< Implementation instance.
};
