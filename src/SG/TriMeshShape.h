#pragma once

#include "Math/TriMesh.h"
#include "Math/Types.h"
#include "SG/Shape.h"
#include "Util/Memory.h"

namespace SG {

DECL_SHARED_PTR(TriMeshShape);

/// Derived Shape class that serves as an abstract base class for all shapes
/// that store a TriMesh to enable intersections.
///
/// \ingroup SG
class TriMeshShape : public Shape {
  public:
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

    /// Returns the TriMesh.
    const TriMesh & GetTriMesh() const { return tri_mesh_; }

  protected:
    /// The constructor is protected to make this abstract.
    TriMeshShape() {}

    /// Implements this to compute the bounds from the mesh.
    virtual Bounds ComputeBounds() const override;

    /// Sets per-face normals in the given Ion shape from the given vector,
    /// which must contain one normal per triangle.
    static void SetFaceNormals(const std::vector<Vector3f> &normals,
                               ion::gfx::Shape &shape);

    /// Sets per-vertex normals in the given Ion shape from the given vector,
    /// which must contain one normal per vertex.
    static void SetVertexNormals(const std::vector<Vector3f> &normals,
                                 ion::gfx::Shape &shape);

    /// Sets per-vertex texture coordinates in the given Ion shape from the
    /// given vector, which must contain one texture coordinate pair per
    /// vertex.
    static void SetTextureCoords(const std::vector<Point2f> &tex_coords,
                                 ion::gfx::Shape &shape);

    /// Adds normals perpendicular to faces to each vertex in the given Ion
    /// shape. The shape should already have room for normals. This works only
    /// for indexed shapes with primitive type kTriangles.
    static void GenerateFaceNormals(ion::gfx::Shape &shape);

    /// Adds normals per vertex computed by averaging over all faces containing
    /// the vertex. The shape should already have room for normals. This works
    /// only for indexed shapes with primitive type kTriangles.
    static void GenerateVertexNormals(ion::gfx::Shape &shape);

    /// Adds texture coordinates to the Ion shape. The type indicates which 2
    /// dimensions should be used to generate the coordinates. For example,
    /// TexCoordsType::kTexCoordsZY indicates that the U texture coordinates
    /// should range from 0 at minimum Z for all points to 1 for maximum Z and
    /// that V texture coordinates should do the same for Y.  The shape should
    /// already have room for texture coordinates. This works only for indexed
    /// shapes with primitive type kTriangles.
    static void GenerateTexCoords(TexCoordsType type, ion::gfx::Shape &shape);

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
