#pragma once

#include <vector>

#include "Base/Memory.h"
#include "SG/TriMeshShape.h"

namespace Parser { class Registry; }

namespace SG {

DECL_SHARED_PTR(MutableTriMeshShape);

/// MutableTriMeshShape is a derived TriMeshShape that allows the stored
/// TriMesh to be changed via API.
///
/// \ingroup SG
class MutableTriMeshShape : public TriMeshShape {
  public:
    /// Changes the mesh in the MutableTriMeshShape to the given one. This
    /// should not be called before Ion is set up.
    void ChangeMesh(const TriMesh &mesh);

    /// Same as ChangeMesh(), but also allows normals and texture coordinates
    /// to be added to the Ion shape. If either vector is not empty, the
    /// supplied values are stored in the shape; there must be one value per
    /// vertex supplied.
    void ChangeMeshWithVertexData(const TriMesh &mesh,
                                  const std::vector<Vector3f> &normals,
                                  const std::vector<Point2f> &tex_coords);

    /// Returns the current TriMesh.
    const TriMesh & GetMesh() const { return GetTriMesh(); }

    /// Allows instances to be copied for cloning.
    void CopyFrom(const MutableTriMeshShape &from);

  protected:
    MutableTriMeshShape() {}

    virtual ion::gfx::ShapePtr CreateSpecificIonShape() override;

  private:
    friend class Parser::Registry;
};

}  // namespace SG
