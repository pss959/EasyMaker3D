#pragma once

#include <X11/Xlib.h>
#include <GL/glx.h>

#include <memory>

class GFX {
  public:
    GFX(int width, int height);
    ~GFX();

    // Query.
    Display     * GetDisplay()  const;
    GLXContext    GetContext()  const;
    GLXDrawable   GetDrawable() const;

    // Rendering.
    void Draw() const;

  private:
    class Helper_;
    std::unique_ptr<Helper_> helper_;
};
