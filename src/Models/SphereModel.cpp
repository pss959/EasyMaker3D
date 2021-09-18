#include "Models/SphereModel.h"

#include "Math/MeshBuilding.h"

TriMesh SphereModel::BuildMesh() {
    // Determine the number of rings and sectors based on the complexity.
    const float complexity = GetComplexity();
    const int num_rings   = 1 + static_cast<int>(complexity * 59);
    const int num_sectors = 3 + static_cast<int>(complexity * 57);
    return BuildSphereMesh(1, num_rings, num_sectors);
}
