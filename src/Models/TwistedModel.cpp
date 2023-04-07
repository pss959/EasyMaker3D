#include "Models/TwistedModel.h"

#include <ion/math/vectorutils.h>

#include "Math/MeshUtils.h"
#include "Util/Assert.h"

void TwistedModel::AddFields() {
    AddModelField(plane_normals_.Init("plane_normals"));

    ConvertedModel::AddFields();
}

void TwistedModel::AddPlaneNormal(const Vector3f &object_normal) {
    auto &normals = plane_normals_.GetValue();
    normals.push_back(ion::math::Normalized(object_normal));
    plane_normals_ = normals;
    ProcessChange(SG::Change::kGeometry, *this);
}

void TwistedModel::RemoveLastPlaneNormal() {
    auto &normals = plane_normals_.GetValue();
    ASSERT(! normals.empty());
    normals.pop_back();
    plane_normals_ = normals;
    ProcessChange(SG::Change::kGeometry, *this);
}

TriMesh TwistedModel::BuildMesh() {
    // Twist the untransformed original mesh.
    ASSERT(GetOriginalModel());
    TriMesh mesh = GetOriginalModel()->GetMesh();
    for (const auto &normal: GetPlaneNormals())
        mesh = MirrorMesh(mesh, Plane(0, normal));
    return mesh;
}
