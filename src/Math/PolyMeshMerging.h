#pragma once

struct PolyMesh;

//! Merges all coplanar faces in the given PolyMesh.
void MergeCoplanarFaces(PolyMesh &poly_mesh);
