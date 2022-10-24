#include "Models/ClippedModel.h"

#include "Math/MeshCombining.h"
#include "Math/MeshUtils.h"
#include "Util/Assert.h"

void ClippedModel::AddFields() {
    AddModelField(planes_.Init("planes"));

    ConvertedModel::AddFields();
}

void ClippedModel::AddPlane(const Plane &local_plane) {
    auto &planes = planes_.GetValue();
    planes.push_back(local_plane);
    planes_ = planes;
    ProcessChange(SG::Change::kGeometry, *this);
}

void ClippedModel::RemoveLastPlane() {
    auto &planes = planes_.GetValue();
    ASSERT(! planes.empty());
    planes.pop_back();
    planes_ = planes;
    ProcessChange(SG::Change::kGeometry, *this);
}

TriMesh ClippedModel::ConvertMesh(const TriMesh &original_mesh) {
    // The original_mesh is in the local coordinates of the original model,
    // which is the same as the object coordinates of the ClippedModel.
    // Clip with the local planes, since the transformations applied to the
    // ClippedModel should affect the resulting mesh.

    TriMesh mesh = original_mesh;
    for (const auto &plane: GetPlanes())
        mesh = ClipMesh(mesh, plane);

    return mesh;
}
