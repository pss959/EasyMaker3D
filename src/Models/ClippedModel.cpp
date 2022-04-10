#include "Models/ClippedModel.h"

#include "Math/MeshCombining.h"
#include "Util/Assert.h"

void ClippedModel::AddFields() {
    AddField(planes_);
    ConvertedModel::AddFields();
}

void ClippedModel::AddPlane(const Plane &plane) {
    auto &planes = planes_.GetValue();
    planes.push_back(plane);
    planes_ = planes;
    ProcessChange(SG::Change::kGeometry, *this);
}

void ClippedModel::RemoveLastPlane() {
    auto &planes = planes_.GetValue();
    ASSERT(planes.size() > 1U);
    planes.pop_back();
    planes_ = planes;
    ProcessChange(SG::Change::kGeometry, *this);
}

TriMesh ClippedModel::BuildMesh() {
    ASSERT(GetOriginalModel());

    TriMesh mesh = GetOriginalLocalMesh();
    for (const auto &plane: GetPlanes())
        mesh = ClipMesh(mesh, plane);

    return mesh;
}
