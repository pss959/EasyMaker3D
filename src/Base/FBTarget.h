#pragma once

#include <ion/gfx/framebufferobject.h>

/// The FBTarget struct packages up framebuffer target information for
/// rendering.
///
/// \ingroup Base
struct FBTarget {
    int target_fb = -1;  ///< Target framebuffer index.
    int color_fb  = -1;  ///< Source framebuffer index for color texture.
    int depth_fb  = -1;  ///< Source framebuffer index for depth texture.

    // XXXX Version 1 used for OpenVR.
    int depth_buffer_id        = -1;
    int render_texture_id      = -1;
    int render_framebuffer_id  = -1;
    int resolve_texture_id     = -1;
    int resolve_framebuffer_id = -1;

    // XXXX Version 2 used for OpenVR.
    ion::gfx::FramebufferObjectPtr rend_fbo;
    ion::gfx::FramebufferObjectPtr dest_fbo;
};
