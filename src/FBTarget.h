#pragma once

/// The FBTarget struct packages up framebuffer target information for
/// rendering.
struct FBTarget {
    int target_fb = -1;  ///< Target framebuffer index.
    int color_fb  = -1;  ///< Source framebuffer index for color texture.
    int depth_fb  = -1;  ///< Source framebuffer index for depth texture.
};
