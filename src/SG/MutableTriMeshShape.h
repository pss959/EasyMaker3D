#pragma once

#include "Memory.h"
#include "SG/TriMeshShape.h"

namespace Parser { class Registry; }

namespace SG {

DECL_SHARED_PTR(MutableTriMeshShape);

/// MutableTriMeshShape is a derived TriMeshShape that allows the stored
/// TriMesh to be changed via API.
class MutableTriMeshShape : public TriMeshShape {
  public:
    /// Changes the mesh in the MutableTriMeshShape to the given one.
    void ChangeMesh(const TriMesh &mesh);

    /// Returns the current TriMesh.
    const TriMesh & GetMesh() const { return GetTriMesh(); }

  protected:
    MutableTriMeshShape() {}

    virtual ion::gfx::ShapePtr CreateSpecificIonShape() override;

  private:
    friend class Parser::Registry;
};

}  // namespace SG
