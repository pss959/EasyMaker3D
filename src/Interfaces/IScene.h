#pragma once

#include <ion/gfx/node.h>
#include <ion/math/matrix.h>
#include <ion/math/range.h>

#include "Interfaces/IInterfaceBase.h"

//! Abstract Interface class defining a scene: everything that is rendered.
//! \ingroup Interfaces
class IScene : public IInterfaceBase {
  public:
    // Sets the projecton matrix to use for the scene.
    virtual void SetProjection(const ion::math::Matrix4f &proj) = 0;

    // Sets the view matrix to use for the scene.
    virtual void SetView(const ion::math::Matrix4f &view) = 0;

    // Sets the viewport to use for the scene.
    virtual void SetViewport(const ion::math::Range2i &viewport) = 0;

    // Returns the root node of the scene.
    virtual const ion::gfx::NodePtr &GetRoot() const = 0;
};
