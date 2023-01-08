#include "SG/Torus.h"

#include "Math/MeshBuilding.h"
#include "Math/MeshUtils.h"

namespace SG {

void Torus::AddFields() {
    AddField(outer_radius_.Init("outer_radius",  1));
    AddField(inner_radius_.Init("inner_radius",  .1f));
    AddField(ring_count_.Init("ring_count",      20));
    AddField(sector_count_.Init("sector_count",  20));

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

void Torus::SetGeometry(float inner_radius, float outer_radius,
                        size_t ring_count, size_t sector_count) {
    inner_radius_ = inner_radius;
    outer_radius_ = outer_radius;
    ring_count_   = ring_count;
    sector_count_ = sector_count;
    UpdateIonShape_();
    ProcessChange(Change::kGeometry, *this);
}

Bounds Torus::ComputeBounds() const {
    const float outer_diameter = 2 * outer_radius_;
    return Bounds(Vector3f(outer_diameter, 2 * inner_radius_, outer_diameter));
}

ion::gfx::ShapePtr Torus::CreateSpecificIonShape() {
    const TriMesh mesh = BuildMesh_();
    InstallMesh(mesh);

    // Allocate space for normals to be generated.
    ion::gfx::ShapePtr ion_shape = TriMeshToIonShape(mesh, true, false);
    GenerateVertexNormals(*ion_shape);
    return ion_shape;
}

void Torus::UpdateIonShape_() {
    if (ion::gfx::ShapePtr ion_shape = GetIonShape()) {
        const TriMesh mesh = BuildMesh_();
        InstallMesh(mesh);
        UpdateIonShapeFromTriMesh(mesh, *ion_shape, true, false);
        GenerateVertexNormals(*ion_shape);
    }
}

TriMesh Torus::BuildMesh_() const {
    return BuildTorusMesh(inner_radius_, outer_radius_,
                          ring_count_, sector_count_);
}

}  // namespace SG
