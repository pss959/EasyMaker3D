//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "SG/MutableTriMeshShape.h"

#include <ion/gfxutils/shapeutils.h>

#include "Math/MeshUtils.h"
#include "Math/ModelMesh.h"
#include "Util/Assert.h"

namespace SG {

void MutableTriMeshShape::ChangeMesh(const TriMesh &mesh) {
    InstallMesh(mesh);
    if (auto shape = GetIonShape())
        UpdateIonShapeFromTriMesh(mesh, *shape);
}

void MutableTriMeshShape::ChangeModelMesh(const ModelMesh &mesh,
                                          bool use_face_normals) {
    InstallMesh(mesh);
    if (auto shape = GetIonShape()) {
        UpdateIonShapeFromTriMesh(mesh, *shape, true, true);
        if (use_face_normals)
            SetFaceNormals(mesh.normals, *shape);
        else
            SetVertexNormals(mesh.normals, *shape);
        SetTextureCoords(mesh.tex_coords, *shape);
    }
}

void MutableTriMeshShape::CopyFrom(const MutableTriMeshShape &from) {
    CopyContentsFrom(from, false);
}

ion::gfx::ShapePtr MutableTriMeshShape::CreateSpecificIonShape() {
    return TriMeshToIonShape(GetTriMesh());
}

}  // namespace SG
