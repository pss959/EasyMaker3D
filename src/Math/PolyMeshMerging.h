//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

/// \file
/// This file defines functions for use in merging PolyMesh structures.
///
/// \ingroup Math

struct PolyMesh;

/// Merges all coplanar faces in the given PolyMesh.
void MergeCoplanarFaces(PolyMesh &poly_mesh);

/// Merges all duplicate vertices, edges, and faces in \c poly_mesh, storing
/// the resulting PolyMesh in \c result_mesh. Both parameters may refer to the
/// same PolyMesh.
void MergeDuplicateFeatures(const PolyMesh &poly_mesh, PolyMesh &result_mesh);
