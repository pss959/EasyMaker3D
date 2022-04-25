#include "SG/MutableTriMeshShape.h"

#include <ion/gfxutils/shapeutils.h>

#include "Math/MeshUtils.h"

namespace SG {

void MutableTriMeshShape::ChangeMesh(const TriMesh &mesh) {
    InstallMesh(mesh);
    if (GetIonShape())
        UpdateIonShapeFromTriMesh(mesh, *GetIonShape());
}

void MutableTriMeshShape::CopyFrom(const MutableTriMeshShape &from) {
    CopyContentsFrom(from, false);
}

ion::gfx::ShapePtr MutableTriMeshShape::CreateSpecificIonShape() {
    return TriMeshToIonShape(GetTriMesh());
}

}  // namespace SG
