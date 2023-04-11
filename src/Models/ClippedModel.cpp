#include "Models/ClippedModel.h"

#include <ion/math/transformutils.h>

#include "Math/MeshCombining.h"
#include "Math/MeshUtils.h"

#include "Util/Assert.h"

void ClippedModel::AddFields() {
    AddModelField(plane_.Init("plane", Plane(0, Vector3f::AxisY())));

    ConvertedModel::AddFields();
}

void ClippedModel::SetPlane(const Plane &plane) {
    plane_ = plane;
    AdjustTranslation_();
    ProcessChange(SG::Change::kGeometry, *this);
}

TriMesh ClippedModel::BuildMesh() {
    // Clip against the untransformed original mesh.
    ASSERT(GetOriginalModel());
    TriMesh mesh = GetOriginalModel()->GetMesh();

    mesh = ClipMesh(mesh, plane_);

    // Set the mesh_offset_ based on the new center.
    mesh_offset_ = Point3f::Zero() - ComputeMeshBounds(mesh).GetCenter();
    AdjustTranslation_();

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

void ClippedModel::AdjustTranslation_() {
    if (mesh_offset_ != Vector3f::Zero()) {
        const Vector3f offset = GetModelMatrix() * mesh_offset_;
        SetTranslation(GetOriginalModel()->GetTranslation() - offset);
    }
}
