#include "Models/HullModel.h"

#include "Math/MeshCombining.h"

TriMesh HullModel::BuildMesh() {
    // Combine the meshes.
    TriMesh mesh = CombineMeshes(GetChildMeshes(),
                                 MeshCombiningOperation::kConvexHull);
    return CenterAndOffsetMesh(mesh);
}
