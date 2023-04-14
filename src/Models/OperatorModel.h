#pragma once

#include <vector>

#include "Base/Memory.h"
#include "Models/ParentModel.h"

DECL_SHARED_PTR(OperatorModel);

/// OperatorModel is an abstract base class for Model classes that operate on
/// one or more operand Model instances. The mesh for a derived OperatorModel
/// is typically created from the mesh or meshes of the operand Model(s). The
/// resulting mesh is recentered on the origin and the centering offset is
/// stored.
///
/// \ingroup Models
class OperatorModel : public ParentModel {
  public:
    /// Returns the offset vector used to center the OperatorModel's mesh. This
    /// vector translates the mesh center to its original position. If the \p
    /// update_first flag is true, this first makes sure the center offset is
    /// up to date. Otherwise, it just returns its current value. (Passing
    /// false can be useful to get the offset during the process of mesh
    /// construction.)
    const Vector3f & GetCenterOffset(bool update_first = true) const;

  protected:
    /// Redefines this to also mark the mesh as stale when a child has been
    /// transformed.
    virtual bool ProcessChange(SG::Change change, const Object &obj) override;

    /// Defines this to call BuildMeshFromOperands() on the derived class and
    /// then recenter the resulting mesh.
    virtual TriMesh BuildMesh() override;

    /// Derived classes must define this to create the mesh from the operand
    /// Model(s).
    virtual TriMesh BuildMeshFromOperands() = 0;

  private:
    /// Offset added to the mesh in object coordinates to center it.
    Vector3f center_offset_{0, 0, 0};
};
