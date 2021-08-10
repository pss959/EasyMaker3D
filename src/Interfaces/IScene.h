#pragma once

#include "Interfaces/IInterfaceBase.h"

struct View;

//! Abstract Interface class defining a scene: everything that is rendered.
//! \ingroup Interfaces
class IScene : public IInterfaceBase {
  public:
    //! Updates the scene based on values in a View instance.
    virtual void UpdateFromView(const View &view) = 0;

    //! Returns the root node of the scene.
    virtual const ion::gfx::NodePtr &GetRoot() const = 0;

    //! Prints the contents of the scene to stdout for debugging.
    virtual void PrintScene() const = 0;
};
