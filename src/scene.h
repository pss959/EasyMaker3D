#pragma once

#include <memory>

#include <ion/gfx/node.h>
#include <ion/math/matrix.h>
#include <ion/math/range.h>

//-----------------------------------------------------------------------------
// A Scene encapsulates everything that is drawn in the window.
//-----------------------------------------------------------------------------

class Scene {
  public:
    Scene();
    ~Scene();

    // Sets the projecton matrix to use for the scene.
    void SetProjection(const ion::math::Matrix4f &proj);

    // Sets the view matrix to use for the scene.
    void SetView(const ion::math::Matrix4f &view);

    // Sets the viewport to use for the scene.
    void SetViewport(const ion::math::Range2i &viewport);

    // Returns the root node of the scene.
    const ion::gfx::NodePtr &GetRoot() const;

  private:
    class Helper_;
    std::unique_ptr<Helper_> helper_;
};
