#pragma once

#include <ion/gfx/node.h>
#include <ion/gfx/statetable.h>

#include "Interfaces/IScene.h"

//! Scene is an implementation of the IScene interface.
class Scene : public IScene {
  public:
    Scene();
    virtual ~Scene();

    // ------------------------------------------------------------------------
    // IScene interface.
    // ------------------------------------------------------------------------
    virtual const char * GetClassName() const override { return "Scene"; }
    virtual void UpdateFromView(const View &view) override;
    virtual const ion::gfx::NodePtr &GetRoot() const override {
        return scene_root_;
    }
    virtual void PrintScene() const override;

  private:
    ion::gfx::StateTablePtr state_table_;
    ion::gfx::NodePtr       scene_root_;
    size_t                  proj_index_;  //!< Index of the projection uniform.
    size_t                  view_index_;  //!< Index of the view uniform.

    //! Builds the Ion StateTable used in the Scene.
    void BuildStateTable_();

    //! Builds the Ion graph representing the Scene.
    void BuildGraph_();
};
