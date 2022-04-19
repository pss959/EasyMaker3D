#include "Models/HullModel.h"

#include "Math/MeshCombining.h"

TriMesh HullModel::BuildMesh() {
    return CombineMeshes(GetChildMeshes(), MeshCombiningOperation::kConvexHull);
}
