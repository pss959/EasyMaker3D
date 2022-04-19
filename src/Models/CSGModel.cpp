#include "Models/CSGModel.h"

#include "Math/MeshCombining.h"

void CSGModel::AddFields() {
    AddField(operation_);
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
    return CombineMeshes(GetChildMeshes(), op);
}
