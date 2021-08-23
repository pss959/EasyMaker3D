#pragma once

#include <ion/gfx/node.h>
#include <ion/math/matrix.h>
#include <ion/math/range.h>

#include "Frustum.h"

//! This class represents a view used to render a scene. It holds an Ion Node
//! that maintains the uniforms used to view the scene; any supplied Nodes are
//! added as children to this Node.
class View {
  public:
    typedef ion::math::Range2i Viewport;  //!< Shorthand.

    View();
    ~View();

    //! Clears the current viewed scene.
    void ClearNodes();

    //! Adds an Ion Node to be viewed.
    void AddNode(const ion::gfx::NodePtr &node);

    //! Sets the viewport used for viewing to the given rectangle.
    void SetViewport(const Viewport &viewport);

    //! Returns the current viewport.
    const Viewport & GetViewport() const { return viewport_; }

    //! Sets the view frustum.
    void SetFrustum(const Frustum &frustum);

    //! Returns the current frustum.
    const Frustum & GetFrustum() const { return frustum_; }

    //! Returns the aspect ratio, based on the current viewport;
    float GetAspectRatio() const;

    //! Returns the Node representing the viewed scene, including viewing
    //! transforms.
    const ion::gfx::NodePtr & GetRoot() const { return root_; }

  private:
    ion::gfx::NodePtr root_;            //!< Root node containing view info.
    Viewport          viewport_;        //!< Current viewport rectangle.
    Frustum           frustum_;         //!< Current view frustum.
    size_t            proj_index_;      //!< Index of the projection uniform.
    size_t            view_index_;      //!< Index of the view uniform.
    size_t            viewport_index_;  //!< Index of the viewport size uniform.

    //! Builds the Ion graph representing the View.
    void BuildGraph_();

    //! Computes and returns a projection matrix given a Frustum
    static ion::math::Matrix4f ComputeProjectionMatrix_(const Frustum &frustum);

    //! Computes and returns a view matrix given a Frustum.
    static ion::math::Matrix4f ComputeViewMatrix_(const Frustum &frustum);
};
