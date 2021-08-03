#pragma once

#include <ion/gfx/node.h>
#include <ion/gfx/statetable.h>

#include "Interfaces/IScene.h"

//! Scene is an implementation of the IScene interface.
class Scene : public IScene {
  public:
    Scene();
    ~Scene();

    virtual void SetProjection(const ion::math::Matrix4f &proj) override;
    virtual void SetView(const ion::math::Matrix4f &view) override;
    virtual void SetViewport(const ion::math::Range2i &viewport) override;
    virtual const ion::gfx::NodePtr &GetRoot() const override {
        return scene_root_;
    }

  private:
    ion::gfx::StateTablePtr state_table_;
    ion::gfx::NodePtr       scene_root_;
    size_t                  proj_index_;
    size_t                  view_index_;

    //! Builds the Ion StateTable used in the Scene.
    void BuildStateTable_();
    //! Builds the Ion graph representing the Scene.
    void BuildGraph_();
};
