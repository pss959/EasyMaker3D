#pragma once

#include <ostream>
#include <string>

#include "Math/Types.h"

//! \name Mesh Utilities
//!@{

// ----------------------------------------------------------------------------
// Mesh transformations.
// ----------------------------------------------------------------------------

//! Applies the given Matrix4x4 to a mesh and returns the result. Note that
//! this does NOT recenter the transformed mesh.
TriMesh TransformMesh(const TriMesh &mesh, const Matrix4f &m);

//! Mirrors a mesh about a plane. Returns the mirrored mesh.
TriMesh MirrorMesh(const TriMesh &mesh, const Plane &plane);

//! Modifies vertices in a mesh if necessary so that the mesh bounds are
//! centered on the origin. Returns the offset vector used to do the
//! recentering.
Vector3f CenterMesh(TriMesh &mesh);

// ----------------------------------------------------------------------------
// I/O.
// ----------------------------------------------------------------------------

//! Writes the given TriMesh in OFF format to the given stream. The description
//! is written as a comment. Note: OFF does not contain normals or texture
//! coords.
void WriteMeshAsOFF(const TriMesh &mesh, const std::string &description,
                    std::ostream &out);

// ----------------------------------------------------------------------------
// Other mesh utilities.
// ----------------------------------------------------------------------------

Bounds ComputeMeshBounds(const TriMesh &mesh);

//!@}
