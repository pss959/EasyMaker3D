#pragma once

/// \file
/// This file defines functions for use in merging PolyMesh structures.
///
/// \ingroup Math

struct PolyMesh;

/// Merges all coplanar faces in the given PolyMesh.
void MergeCoplanarFaces(PolyMesh &poly_mesh);

/// Merges all duplicate vertices edges, and faces in the given PolyMesh to
/// guarantee uniqueness.
void MergeDuplicateFeatures(PolyMesh &poly_mesh);
