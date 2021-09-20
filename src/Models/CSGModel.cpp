#include "Models/CSGModel.h"

#include "Math/CGALInterface.h"
#include "SG/Exception.h"
#include "Util/String.h"

void CSGModel::AddFields() {
    AddField(operation_);
    CombinedModel::AddFields();
}

void CSGModel::AllFieldsParsed() {
    CombinedModel::AllFieldsParsed();
    if (GetOperandModels().size() < 2U)
        ThrowReadError("Only " + Util::ToString(GetOperandModels().size()) +
                       " operand models; at least 2 required");
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
