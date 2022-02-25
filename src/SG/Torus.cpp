#include "SG/Torus.h"

#include "Math/MeshBuilding.h"
#include "Math/MeshUtils.h"

namespace SG {

void Torus::AddFields() {
    AddField(outer_radius_);
    AddField(inner_radius_);
    AddField(ring_count_);
    AddField(sector_count_);
    TriMeshShape::AddFields();
}

void Torus::SetInnerRadius(float radius) {
    inner_radius_ = radius;
    UpdateIonShape_();
    ProcessChange(Change::kGeometry, *this);
}

void Torus::SetOuterRadius(float radius) {
    outer_radius_ = radius;
    UpdateIonShape_();
    ProcessChange(Change::kGeometry, *this);
}

Bounds Torus::ComputeBounds() const {
    const float outer_diameter = 2 * (outer_radius_ + inner_radius_);
    return Bounds(Vector3f(outer_diameter, 2 * inner_radius_, outer_diameter));
}

ion::gfx::ShapePtr Torus::CreateSpecificIonShape() {
    const TriMesh mesh = BuildTorusMesh(inner_radius_, outer_radius_,
                                        ring_count_, sector_count_);
    InstallMesh(mesh);

    // Allocate space for normals to be generated.
    ion::gfx::ShapePtr ion_shape = TriMeshToIonShape(mesh, true, false);
    GenerateNormals(*ion_shape, NormalType::kVertexNormals);
    return ion_shape;
}

void Torus::UpdateIonShape_() {
    if (ion::gfx::ShapePtr ion_shape = GetIonShape()) {
        const TriMesh mesh = BuildMesh_();
        InstallMesh(mesh);
        UpdateIonShapeFromTriMesh(mesh, *ion_shape, true, false);
        GenerateNormals(*ion_shape, NormalType::kVertexNormals);
    }
}

TriMesh Torus::BuildMesh_() const {
    return BuildTorusMesh(inner_radius_, outer_radius_,
                          ring_count_, sector_count_);
}

}  // namespace SG
