#include "Models/MirroredModel.h"

#include "Math/MeshUtils.h"
#include "Util/Assert.h"

void MirroredModel::AddFields() {
    AddModelField(planes_.Init("planes"));

    ConvertedModel::AddFields();
}

void MirroredModel::AddPlane(const Plane &local_plane) {
    auto &planes = planes_.GetValue();
    planes.push_back(local_plane);
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

TriMesh MirroredModel::BuildMesh() {
    // Mirror the untransformed original mesh.
    ASSERT(GetOriginalModel());
    TriMesh mesh = GetOriginalModel()->GetMesh();
    for (const auto &plane: GetPlanes())
        mesh = MirrorMesh(mesh, plane);
    return mesh;
}
