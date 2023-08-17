#include "Models/ClippedModel.h"

#include "Math/Linear.h"
#include "Math/MeshClipping.h"

void ClippedModel::AddFields() {
    AddModelField(plane_.Init("plane", GetDefaultPlane()));

    ConvertedModel::AddFields();
}

bool ClippedModel::IsValid(Str &details) {
    if (! ConvertedModel::IsValid(details))
        return false;
    if (! IsValidVector(GetPlane().normal)) {
        details = "Zero-length plane normal";
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
