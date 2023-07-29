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

void MutableTriMeshShape::ChangeModelMesh(const ModelMesh &mesh,
                                          bool use_face_normals) {
    InstallMesh(mesh);
    if (! GetIonShape())
        SetUpIon();
    auto &shape = *GetIonShape();
    UpdateIonShapeFromTriMesh(mesh, shape, true, true);
    if (use_face_normals)
        SetFaceNormals(mesh.normals, shape);
    else
        SetVertexNormals(mesh.normals, shape);
    SetTextureCoords(mesh.tex_coords, shape);
}

void MutableTriMeshShape::CopyFrom(const MutableTriMeshShape &from) {
    CopyContentsFrom(from, false);
}

ion::gfx::ShapePtr MutableTriMeshShape::CreateSpecificIonShape() {
    return TriMeshToIonShape(GetTriMesh());
}

}  // namespace SG
