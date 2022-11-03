#include "Models/HullModel.h"

#include "Math/MeshCombining.h"

TriMesh HullModel::CombineMeshes(const std::vector<TriMesh> &meshes) {
    ASSERT(! meshes.empty());
    return ::CombineMeshes(meshes, MeshCombiningOperation::kConvexHull);
}
