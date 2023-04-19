#include "Models/MirroredModel.h"

#include "Math/Linear.h"
#include "Math/MeshUtils.h"

void MirroredModel::AddFields() {
    AddModelField(plane_.Init("plane", GetDefaultPlane()));

    ConvertedModel::AddFields();
}

bool MirroredModel::IsValid(std::string &details) {
    if (! ConvertedModel::IsValid(details))
        return false;
    if (! IsValidVector(GetPlane().normal)) {
        details = "zero-length plane normal";
        return false;
    }
    return true;
}

void MirroredModel::SetPlane(const Plane &plane) {
    plane_ = plane;
    ProcessChange(SG::Change::kGeometry, *this);
}

TriMesh MirroredModel::ConvertMesh(const TriMesh &mesh) {
    return MirrorMesh(mesh, plane_);
}
