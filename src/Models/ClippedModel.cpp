#include "Models/ClippedModel.h"

#include <ion/math/transformutils.h>

#include "Math/MeshCombining.h"
#include "Math/MeshUtils.h"

#include "Util/Assert.h"

void ClippedModel::AddFields() {
    AddModelField(planes_.Init("planes"));

    ConvertedModel::AddFields();
}

void ClippedModel::AddPlane(const Plane &object_plane) {
    auto &planes = planes_.GetValue();
    planes.push_back(object_plane);
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

TriMesh ClippedModel::BuildMesh() {
    // Clip against the untransformed original mesh.
    ASSERT(GetOriginalModel());
    TriMesh mesh = GetOriginalModel()->GetMesh();

    for (const auto &plane: GetPlanes())
        mesh = ClipMesh(mesh, plane);

    mesh_offset_ = Point3f::Zero() - ComputeMeshBounds(mesh).GetCenter();

    return CenterMesh(mesh);
}

void ClippedModel::SyncTransformsFromOriginal(const Model &original) {
    CopyTransformsFrom(original);
    if (mesh_offset_ != Vector3f::Zero())
        SetTranslation(GetTranslation() - GetModelMatrix() * mesh_offset_);
}

void ClippedModel::SyncTransformsToOriginal(Model &original) const {
    original.CopyTransformsFrom(*this);
    if (mesh_offset_ != Vector3f::Zero())
        original.SetTranslation(original.GetTranslation() +
                                GetModelMatrix() * mesh_offset_);
}
