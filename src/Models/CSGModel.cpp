#include "Models/CSGModel.h"

#include "Math/MeshCombining.h"
#include "Math/MeshUtils.h"
#include "SG/Exception.h"
#include "Util/String.h"

void CSGModel::AddFields() {
    AddField(operation_);
    CombinedModel::AddFields();
}

bool CSGModel::IsValid(std::string &details) {
    if (! CombinedModel::IsValid(details))
        return false;
    if (GetOperandModels().size() < 2U) {
        details = "Only " + Util::ToString(GetOperandModels().size()) +
            " operand models; at least 2 required";
        return false;
    }
    return true;
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

    // Combine the meshes.
    TriMesh mesh = CombineMeshes(GetChildMeshes(), op);

    // Center the mesh on the origin and apply the centering offset as a
    // translation to the CSGModel.
    SetTranslation(-CenterMesh(mesh));

    return mesh;
}
