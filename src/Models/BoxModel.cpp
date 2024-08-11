//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Models/BoxModel.h"

#include "Math/MeshBuilding.h"

TriMesh BoxModel::BuildMesh() {
    return BuildBoxMesh(Vector3f(2, 2, 2));
}
