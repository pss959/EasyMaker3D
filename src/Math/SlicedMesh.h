//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include "Enums/Dim.h"
#include "Math/TriMesh.h"
#include "Math/Types.h"

/// A SlicedMesh represents a TriMesh that has been sliced by some number of
/// planes perpendicular to a given coordinate axis. It saves information that
/// can be used to avoid extra computations for Model types that need to slice
/// during mesh construction.
struct SlicedMesh {
    TriMesh  mesh;
    Dim      axis;
    Range1f  range;
};
