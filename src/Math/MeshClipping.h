//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include "Math/Types.h"

struct Plane;
struct TriMesh;

/// \file
/// This file defines operations for clipping TriMesh instances using CGAL
///
/// \ingroup Math

/// Clips a mesh (in place) by a Plane, returning the resulting mesh.
TriMesh ClipMesh(const TriMesh &mesh, const Plane &plane);
