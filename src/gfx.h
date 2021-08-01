#pragma once

#include <X11/Xlib.h>
#include <GL/glx.h>

#include <memory>

#include <ion/math/matrix.h>
#include <ion/math/range.h>

class GFX {
  public:
    // Information passed to DrawWithInfo().
    struct RenderInfo {
        // Viewport to render to.
        ion::math::Range2i viewport_rect;

        // Projection and view matrices.
        ion::math::Matrix4f projection;
        ion::math::Matrix4f view;

        // Framebuffer indices.
        int fb;        // Target framebuffer.
        int color_fb;  // Source framebuffer for color texture.
        int depth_fb;  // Source framebuffer for depth texture.
    };

    GFX();
    ~GFX();

    // Query.
    Display     * GetDisplay()  const;
    GLXContext    GetContext()  const;
    GLXDrawable   GetDrawable() const;

    // Rendering.
    int  CreateFramebuffer();
    void SetFramebuffer(int buffer);
    void Draw(int width, int height) const;
    void DrawWithInfo(const RenderInfo &info) const;

  private:
    class Helper_;
    std::unique_ptr<Helper_> helper_;
};
