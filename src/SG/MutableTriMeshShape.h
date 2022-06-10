#pragma once

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
    /// Changes the mesh in the MutableTriMeshShape to the given one. The
    /// alloc_normals and alloc_tex_coords parameters can be set to true to
    /// leave room in Ion vertices.
    void ChangeMesh(const TriMesh &mesh, bool alloc_normals = false,
                    bool alloc_tex_coords = false);

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
