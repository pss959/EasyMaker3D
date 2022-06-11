#pragma once

#include <ion/gfx/framebufferobject.h>

/// The FBTarget struct packages up framebuffer information for rendering (for
/// OpenVR).
///
/// \ingroup Base
struct FBTarget {
    /// Multisampled framebuffer the scene is rendered into.
    ion::gfx::FramebufferObjectPtr rendered_fbo;
    /// Standard framebuffer the rendered framebuffer is resolved into.
    ion::gfx::FramebufferObjectPtr resolved_fbo;
};
