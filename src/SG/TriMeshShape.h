#pragma once

#include "SG/Math.h"
#include "SG/Shape.h"

namespace SG {

//! Derived Shape class that serves as an abstract base class for all shapes
//! that store a TriMesh to enable intersections.
class TriMeshShape : public Shape {
  public:
    //! Returns the current Bounds.
    const Bounds & GetBounds();

    //! Implements this to intersect the TriMesh.
    virtual bool IntersectRay(const Ray &ray, Hit &hit) const override;

  protected:
    //! The constructor is protected to make this abstract.
    TriMeshShape() {}

    //! Derived classes can call this to set up the TriMesh struct with
    //! triangles from the given Ion shape.
    void FillTriMesh(const ion::gfx::Shape &shape);

    //! Derived classes can call this to install a TriMesh.
    void InstallMesh(const TriMesh &mesh) { tri_mesh_ = mesh; }

  private:
    //! Triangle mesh for the Ion shape.
    TriMesh tri_mesh_;
};

}  // namespace SG
