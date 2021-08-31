#pragma once

#include "Math/Types.h"

//! This class represents a view used to render a scene. It can return the
//! projection and view matrices representing the view.
class View {
  public:
    typedef ion::math::Range2i Viewport;  //!< Shorthand.

    //! Sets the viewport used for viewing to the given rectangle.
    void SetViewport(const Viewport &viewport) { viewport_ = viewport; }

    //! Returns the current viewport.
    const Viewport & GetViewport() const { return viewport_; }

    //! Sets the view frustum.
    void SetFrustum(const Frustum &frustum) { frustum_ = frustum; }

    //! Returns the current frustum.
    const Frustum & GetFrustum() const { return frustum_; }

    //! Returns the aspect ratio, based on the current viewport;
    float GetAspectRatio() const;

    //! Returns the projection matrix.
    ion::math::Matrix4f GetProjectionMatrix() const;

    //! Returns the view matrix.
    ion::math::Matrix4f GetViewMatrix() const;

  private:
    Viewport viewport_;        //!< Current viewport rectangle.
    Frustum  frustum_;         //!< Current view frustum.
};
