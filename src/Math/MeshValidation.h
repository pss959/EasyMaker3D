//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include <vector>

#include "Math/Types.h"

struct TriMesh;

/// \name Mesh Validation
///
/// \ingroup Math
///@{

/// Possible return codes from ValidateTriMesh().
///
/// \ingroup Math
enum class MeshValidityCode {
    kValid,             ///< The mesh is valid.
    kInconsistent,      ///< The mesh has inconsistent connectivity.
    kNotClosed,         ///< The mesh does not form a closed polyhedron.
    kSelfIntersecting,  ///< The mesh contains self-intersections.
};

/// Returns a code indicating whether the given TriMesh is valid.
MeshValidityCode ValidateTriMesh(const TriMesh &mesh);

/// Returns a code indicating whether the given TriMesh is valid. If it is not
/// valid, this attempts to repair it and tests it again if possible.
MeshValidityCode ValidateAndRepairTriMesh(TriMesh &mesh);

/// If either validation function returns MeshValidityCode::kNotClosed, this
/// can be used to return the border edges. Each edge is a pair of indices in
/// the returned vector.
std::vector<GIndex> GetBorderEdges(const TriMesh &mesh);

/// Computes and returns the volume of the given TriMesh, which must be valid.
float ComputeMeshVolume(const TriMesh &mesh);

///@}
