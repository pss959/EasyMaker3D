#pragma once

#include <ion/gfx/node.h>
#include <ion/math/matrix.h>
#include <ion/math/range.h>

#include "Graph/Typedefs.h"

//! This class represents a view used to render a scene. It holds an Ion Node
//! that maintains the uniforms used to view the scene; the scene's root is
//! added as a child to this Node.
class View {
  public:
    View();
    ~View();

    //! Sets the Scene to be viewed. This sets the view parameters to match the
    //! Camera in the scene.
    void SetScene(const Graph::ScenePtr &scene);

    //! Returns the Scene being viewed.
    const Graph::ScenePtr & GetScene() const { return scene_; }

    //! Updates the viewport used for viewing.
    void UpdateViewport(const ion::math::Range2i &viewport_rect);

    //! Updates the view parameters to match the given Camera.
    void UpdateFromCamera(const Graph::Camera &camera);

    //! Returns the Node representing the viewed scene.
    const ion::gfx::NodePtr & GetRoot() const { return root_; }

    //! Prints the contents of the view to stdout for debugging.
    void PrintContents() const;

  private:
    Graph::ScenePtr   scene_;       //!< Scene being viewed.
    ion::gfx::NodePtr root_;        //!< Root node containing view info.
    size_t            proj_index_;  //!< Index of the projection uniform.
    size_t            view_index_;  //!< Index of the view uniform.

    //! Builds the Ion graph representing the View.
    ion::gfx::NodePtr BuildGraph_();

    //! Computes and returns an Ion projection matrix given a Graph::Camera.
    static ion::math::Matrix4f ComputeProjectionMatrix_(
        const Graph::Camera &camera);

    //! Computes and returns an Ion view matrix given a Graph::Camera.
    static ion::math::Matrix4f ComputeViewMatrix_(const Graph::Camera &camera);
};
