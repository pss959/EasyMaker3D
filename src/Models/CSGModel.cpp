#include "Models/CSGModel.h"

#include "Math/CGALInterface.h"
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
    ProcessChange(SG::Change::kGeometry);
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
        op = MeshCombiningOperation::kCSGDifference;
        break;
    }
    return CombineMeshes(GetChildMeshes(), op);
}
