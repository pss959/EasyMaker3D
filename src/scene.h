#pragma once

#include <memory>

#include <ion/gfx/node.h>

//-----------------------------------------------------------------------------
// A Scene encapsulates everything that is drawn in the window.
//-----------------------------------------------------------------------------

class Scene {
  public:
    Scene();
    ~Scene();

    // Sets the viewport to use for the scene.
    void SetViewport(const ion::math::Range2i &viewport);

    // Returns the root node of the scene.
    const ion::gfx::NodePtr &GetRoot() const;

  private:
    class Helper_;
    std::unique_ptr<Helper_> helper_;
};
