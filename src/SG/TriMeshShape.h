#pragma once

#include "Math/Types.h"
#include "SG/Shape.h"

namespace SG {

/// Derived Shape class that serves as an abstract base class for all shapes
/// that store a TriMesh to enable intersections.
class TriMeshShape : public Shape {
  public:
    /// Implements this to intersect the TriMesh.
    virtual bool IntersectRay(const Ray &ray, Hit &hit) const override;

  protected:
    /// The constructor is protected to make this abstract.
    TriMeshShape() {}

    /// Returns the TriMesh.
    const TriMesh & GetTriMesh() const { return tri_mesh_; }

    /// Implements this to compute the bounds from the mesh.
    virtual Bounds ComputeBounds() const override;

    /// Adds vertex normals to the Ion shape. A vertex normal is computed by
    /// averaging over all faces containing the vertex. The shape should
    /// already have room for normals. This works only for indexed shapes with
    /// primitive type kTriangles.
    void GenerateVertexNormals(ion::gfx::Shape &shape);

    /// Adds texture coordinates to the Ion shape. The dimensions vector
    /// indicates which 2 dimensions should be used to generate the
    /// coordinates. For example, dimensions=(2,1) indicates that the U texture
    /// coordinates should range from 0 at minimum Z for all points to 1 for
    /// maximum Z and that V texture coordinates should do the same for Y.  The
    /// shape should already have room for texture coordinates. This works only
    /// for indexed shapes with primitive type kTriangles.
    void GenerateTexCoords(ion::gfx::Shape &shape, const Vector2i &dimensions);

    /// Derived classes can call this to set up the TriMesh struct with
    /// triangles from the given Ion shape.
    void FillTriMesh(const ion::gfx::Shape &shape);

    /// Derived classes can call this to install a TriMesh.
    void InstallMesh(const TriMesh &mesh);

  private:
    /// Triangle mesh for the Ion shape.
    TriMesh tri_mesh_;
};

}  // namespace SG
