#pragma once

#include <ion/gfx/framebufferobject.h>

#include "Math/Types.h"
#include "Util/Memory.h"

/// An FBTarget packages up framebuffer information for offscreen rendering
/// (for OpenVR or script-based applications).
///
/// \ingroup Base
class FBTarget {
  public:
    /// Sets up the target for offscreen rendering to a framebuffer with the
    /// given size and number of multisampled samples. The \p label_prefix is
    /// used to create labels for the framebuffer objects.
    void Init(const Str &prefix, const Vector2ui &size, uint samples);

    /// Returns true if the FBTarget was initialized properly.
    bool IsInitialized() const {
        return rendered_fbo_ && resolved_fbo_;
    }

    /// Returns the rendered framebuffer object.
    const ion::gfx::FramebufferObjectPtr GetRenderedFBO() const {
        return rendered_fbo_;
    }

    /// Returns the resolved framebuffer object.
    const ion::gfx::FramebufferObjectPtr GetResolvedFBO() const {
        return resolved_fbo_;
    }

  private:
    /// Multisampled framebuffer the scene is rendered into.
    ion::gfx::FramebufferObjectPtr rendered_fbo_;
    /// Standard framebuffer the rendered framebuffer is resolved into.
    ion::gfx::FramebufferObjectPtr resolved_fbo_;
};

DECL_SHARED_PTR(FBTarget);
