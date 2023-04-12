#include "Models/ClippedModel.h"

#include <ion/math/transformutils.h>
#include <ion/math/vectorutils.h>

#include "Math/MeshCombining.h"
#include "Math/MeshUtils.h"

#include "Util/Assert.h"

void ClippedModel::AddFields() {
    AddModelField(plane_.Init("plane", Plane(0, Vector3f::AxisY())));

    ConvertedModel::AddFields();
}

void ClippedModel::SetPlane(const Plane &plane) {
    // Store the plane. The mesh offset and offset plane will be updated when
    // the mesh is rebuilt.
    plane_ = plane;
    ProcessChange(SG::Change::kGeometry, *this);
}

TriMesh ClippedModel::BuildMesh() {
    // Clip against the untransformed original mesh.
    ASSERT(GetOriginalModel());
    TriMesh mesh = GetOriginalModel()->GetMesh();

    mesh = ClipMesh(mesh, plane_);

    // Update the mesh_offset_ based on the new center. Use it to update the
    // offset plane and the compensating translation.
    UpdateMeshOffset_(mesh);

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

void ClippedModel::UpdateMeshOffset_(const TriMesh &mesh) {
    mesh_offset_ = Point3f::Zero() - ComputeMeshBounds(mesh).GetCenter();

    // Compute the offset plane. Compensate for the mesh offset in the normal
    // direction. Note that the scale and rotation have to be applied to the
    // offset vector. (This also applies the translation, but translation does
    // not get applied to a Vector3f.)
    const Vector3f offset_vec = GetModelMatrix() * -mesh_offset_;
    offset_plane_ = plane_;
    offset_plane_.distance -= ion::math::Dot(offset_vec, offset_plane_.normal);
    std::cerr << "XXXX P=" << plane_.GetValue().ToString()
              << " OV=" << offset_vec
              << " D=" << ion::math::Dot(offset_vec, offset_plane_.normal)
              << " OP=" << offset_plane_.ToString()
              << "\n";

    // Adjust the translation to keep the mesh in the same relative place as
    // the original Model.
    SetTranslation(GetOriginalModel()->GetTranslation() + offset_vec);
}
