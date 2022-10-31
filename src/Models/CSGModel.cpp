#include "Models/CSGModel.h"

#include "Math/MeshCombining.h"
#include "Math/MeshUtils.h"

void CSGModel::AddFields() {
    AddModelField(operation_.Init("operation", CSGOperation::kUnion));

    CombinedModel::AddFields();
}

void CSGModel::SetOperation(CSGOperation op) {
    operation_ = op;
    ProcessChange(SG::Change::kGeometry, *this);
}

TriMesh CSGModel::BuildMesh() {
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
    return CenterMesh(CombineMeshes(GetChildMeshes(), op));
}
