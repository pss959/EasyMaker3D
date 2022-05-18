#include "Models/MirroredModel.h"

#include "Math/MeshUtils.h"
#include "Util/Assert.h"

void MirroredModel::AddFields() {
    AddModelField(planes_);
    ConvertedModel::AddFields();
}

void MirroredModel::AddPlane(const Plane &plane) {
    auto &planes = planes_.GetValue();
    planes.push_back(plane);
    planes_ = planes;
    ProcessChange(SG::Change::kGeometry, *this);
}

void MirroredModel::RemoveLastPlane() {
    auto &planes = planes_.GetValue();
    ASSERT(! planes.empty());
    planes.pop_back();
    planes_ = planes;
    ProcessChange(SG::Change::kGeometry, *this);
}

TriMesh MirroredModel::ConvertMesh(const TriMesh &original_mesh) {
    TriMesh mesh = original_mesh;
    for (const auto &plane: GetPlanes())
        mesh = MirrorMesh(mesh, plane);
    return mesh;
}
