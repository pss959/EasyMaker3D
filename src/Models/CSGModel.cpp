//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Models/CSGModel.h"

#include "Math/MeshCombining.h"
#include "Math/MeshUtils.h"
#include "Util/Enum.h"
#include "Util/General.h"
#include "Util/KLog.h"
#include "Util/String.h"

void CSGModel::AddFields() {
    AddModelField(operation_.Init("operation", CSGOperation::kUnion));

    CombinedModel::AddFields();
}

void CSGModel::SetOperation(CSGOperation op) {
    operation_ = op;
    ProcessChange(SG::Change::kGeometry, *this);
}

TriMesh CSGModel::CombineMeshes(const std::vector<TriMesh> &meshes) {
    ASSERT(meshes.size() >= 2U);
    KLOG('C', "CSGModel " << GetName() << " applying "
         << Util::EnumName(operation_.GetValue()) << " to "
         << Util::JoinItems(
             Util::ConvertVector<Str, ModelPtr>(
                 GetOperandModels(),
                 [](const ModelPtr &p){ return p->ToString(); })));

    MeshCombiningOperation op;
    switch (operation_) {
      case CSGOperation::kUnion:
        op = MeshCombiningOperation::kCSGUnion;
        break;
      case CSGOperation::kIntersection:
        op = MeshCombiningOperation::kCSGIntersection;
        break;
      case CSGOperation::kDifference:
      default:
        op = MeshCombiningOperation::kCSGDifference;
        break;
    }
    return ::CombineMeshes(meshes, op);
}
