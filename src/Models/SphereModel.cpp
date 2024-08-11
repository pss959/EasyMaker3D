//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Models/SphereModel.h"

#include "Math/Linear.h"
#include "Math/MeshBuilding.h"
#include "Util/Tuning.h"

TriMesh SphereModel::BuildMesh() {
    // Determine the number of rings and sectors based on the complexity.
    const float complexity = GetComplexity();
    const int num_rings   = LerpInt(complexity, 1, TK::kMaxSphereRings);
    const int num_sectors = LerpInt(complexity, 3, TK::kMaxSphereSectors);
    return BuildSphereMesh(1, num_rings, num_sectors);
}
