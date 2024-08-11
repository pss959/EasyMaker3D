//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include <vector>

#include "Models/ParentModel.h"
#include "Util/Memory.h"

DECL_SHARED_PTR(OperatorModel);

/// OperatorModel is an abstract base class for Model classes that operate on
/// one or more operand Model instances. The mesh for a derived OperatorModel
/// is typically created from the mesh or meshes of the operand Model(s). The
/// resulting mesh is recentered on the origin and the centering offset is
/// stored.
///
/// \ingroup Models
class OperatorModel : public ParentModel {
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

    /// Redefines this to not copy child Models.
    virtual void CopyContentsFrom(const Parser::Object &from,
                                  bool is_deep) override;
};
