#pragma once

#include <X11/Xlib.h>
#include <GL/glx.h>

#include <memory>

#include <ion/math/range.h>

class GFX {
  public:
    // XXX Struct to help set up VR context.
    struct RenderInfo {
        ion::math::Range2i viewport_rect;
        int fb;
        int color_fb;
        int depth_fb;
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
