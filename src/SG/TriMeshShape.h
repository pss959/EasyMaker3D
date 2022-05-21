#pragma once

#include "Base/Memory.h"
#include "Math/Types.h"
#include "SG/Shape.h"

namespace SG {

DECL_SHARED_PTR(TriMeshShape);

/// Derived Shape class that serves as an abstract base class for all shapes
/// that store a TriMesh to enable intersections.
///
/// \ingroup SG
class TriMeshShape : public Shape {
  public:
    /// Indicates how to generate surface normals for the shape.
    enum class NormalType {
        kNoNormals,      ///< Do not generate normals.
        kVertexNormals,  ///< Generate normals smoothed at vertices.
        kFaceNormals,    ///< Generate normals based on faces.
    };

    /// Indicates how to generate texture coordinates for the shape.
    enum class TexCoordsType {
        kNoTexCoords,  ///< Do not generate texture coordinates.
        kTexCoordsXY,  ///< Generate texture coordinates using X/Y dimensions.
        kTexCoordsXZ,  ///< Generate texture coordinates using X/Z dimensions.
        kTexCoordsYX,  ///< Generate texture coordinates using Y/X dimensions.
        kTexCoordsYZ,  ///< Generate texture coordinates using Y/Z dimensions.
        kTexCoordsZX,  ///< Generate texture coordinates using Z/X dimensions.
        kTexCoordsZY,  ///< Generate texture coordinates using Z/Y dimensions.
    };

    /// Implements this to intersect the TriMesh.
    virtual bool IntersectRay(const Ray &ray, Hit &hit) const override;

  protected:
    /// The constructor is protected to make this abstract.
    TriMeshShape() {}

    /// Returns the TriMesh.
    const TriMesh & GetTriMesh() const { return tri_mesh_; }

    /// Implements this to compute the bounds from the mesh.
    virtual Bounds ComputeBounds() const override;

    /// Adds normals of the given type to the Ion shape. If the type is
    /// NormalType::kVertexNormals, each normal is computed by averaging over
    /// all faces containing the vertex. If it is NormalType::kFaceNormals,
    /// normals are all perpendicular to their faces. The shape should already
    /// have room for normals. This works only for indexed shapes with
    /// primitive type kTriangles.
    static void GenerateNormals(ion::gfx::Shape &shape, NormalType type);

    /// Adds texture coordinates to the Ion shape. The type indicates which 2
    /// dimensions should be used to generate the coordinates. For example,
    /// TexCoordsType::kTexCoordsZY indicates that the U texture coordinates
    /// should range from 0 at minimum Z for all points to 1 for maximum Z and
    /// that V texture coordinates should do the same for Y.  The shape should
    /// already have room for texture coordinates. This works only for indexed
    /// shapes with primitive type kTriangles.
    static void GenerateTexCoords(ion::gfx::Shape &shape, TexCoordsType type);

    /// Derived classes can call this to set up the TriMesh struct with
    /// triangles from the given Ion shape.
    void FillTriMesh(const ion::gfx::Shape &shape);

    /// Derived classes can call this to install a TriMesh.
    void InstallMesh(const TriMesh &mesh);

    /// Redefines this to copy the mesh.
    virtual void CopyContentsFrom(const Parser::Object &from,
                                  bool is_deep) override;

  private:
    /// Triangle mesh for the Ion shape.
    TriMesh tri_mesh_;
};

}  // namespace SG
