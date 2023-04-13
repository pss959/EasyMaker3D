#include "Models/ClippedModel.h"

#include "Math/Linear.h"
#include "Math/MeshCombining.h"

void ClippedModel::AddFields() {
    AddModelField(plane_.Init("plane", Plane(0, Vector3f::AxisY())));

    ConvertedModel::AddFields();
}

bool ClippedModel::IsValid(std::string &details) {
    if (! ConvertedModel::IsValid(details))
        return false;
    if (! IsValidVector(GetPlane().normal)) {
        details = "zero-length plane normal";
        return false;
    }
    return true;
}

void ClippedModel::SetPlane(const Plane &plane) {
    plane_ = plane;
    ProcessChange(SG::Change::kGeometry, *this);
}

TriMesh ClippedModel::ConvertMesh(const TriMesh &mesh) {
    return ClipMesh(mesh, plane_);
}
