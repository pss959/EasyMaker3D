#include "Models/HullModel.h"

#include "Math/MeshCombining.h"
#include "Math/MeshUtils.h"

TriMesh HullModel::BuildMesh() {
    return CenterMesh(CombineMeshes(GetChildMeshes(),
                                    MeshCombiningOperation::kConvexHull));
}
