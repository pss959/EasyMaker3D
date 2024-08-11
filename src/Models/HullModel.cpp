//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Models/HullModel.h"

#include "Math/MeshCombining.h"

TriMesh HullModel::CombineMeshes(const std::vector<TriMesh> &meshes) {
    ASSERT(! meshes.empty());
    return ::CombineMeshes(meshes, MeshCombiningOperation::kConvexHull);
}
