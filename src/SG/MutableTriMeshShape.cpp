#include "SG/MutableTriMeshShape.h"

#include <ion/gfxutils/shapeutils.h>

#include "Math/MeshUtils.h"

namespace SG {

void MutableTriMeshShape::ChangeMesh(const TriMesh &mesh) {
    InstallMesh(mesh);
    if (! GetIonShape())
        SetUpIon();
    UpdateIonShapeFromTriMesh(mesh, *GetIonShape());
}

void MutableTriMeshShape::ChangeMeshWithVertexData(
    const TriMesh &mesh, const std::vector<Vector3f> &normals,
    const std::vector<Point2f> &tex_coords) {
    if (! GetIonShape())
        SetUpIon();

    const bool has_normals    = ! normals.empty();
    const bool has_tex_coords = ! tex_coords.empty();

    InstallMesh(mesh);
    if (! GetIonShape())
        SetUpIon();
    auto &shape = *GetIonShape();
    UpdateIonShapeFromTriMesh(mesh, shape, has_normals, has_tex_coords);
    if (has_normals)
        SetVertexNormals(normals, shape);
    if (has_tex_coords)
        SetTextureCoords(tex_coords, shape);
}

void MutableTriMeshShape::CopyFrom(const MutableTriMeshShape &from) {
    CopyContentsFrom(from, false);
}

ion::gfx::ShapePtr MutableTriMeshShape::CreateSpecificIonShape() {
    return TriMeshToIonShape(GetTriMesh());
}

}  // namespace SG
