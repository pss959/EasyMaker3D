#include "Models/MirroredModel.h"

#include "Math/Linear.h"
#include "Math/MeshUtils.h"

void MirroredModel::AddFields() {
    AddModelField(plane_normal_.Init("plane_normal", GetDefaultPlaneNormal()));

    ConvertedModel::AddFields();
}

bool MirroredModel::IsValid(std::string &details) {
    if (! ConvertedModel::IsValid(details))
        return false;
    if (! IsValidVector(GetPlaneNormal())) {
        details = "zero-length plane normal";
        return false;
    }
    return true;
}

void MirroredModel::SetPlaneNormal(const Vector3f &plane_normal) {
    plane_normal_ = plane_normal;
    ProcessChange(SG::Change::kGeometry, *this);
}

void MirroredModel::SyncTransformsFromOperand(const Model &operand) {
    // Leave the translation alone.
    SetScale(operand.GetScale());
    SetRotation(operand.GetRotation());
}

void MirroredModel::SyncTransformsToOperand(Model &operand) const {
    // Leave the translation alone.
    operand.SetScale(GetScale());
    operand.SetRotation(GetRotation());
}

TriMesh MirroredModel::ConvertMesh(const TriMesh &mesh) {
    return MirrorMesh(mesh, Plane(0, plane_normal_));
}
