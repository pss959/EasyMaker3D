#pragma once

#include <ostream>
#include <string>

#include <ion/gfx/shape.h>

#include "Math/Types.h"

/// \name Mesh Utilities
///@{

// ----------------------------------------------------------------------------
// Mesh transformations.
// ----------------------------------------------------------------------------

/// Applies the given Matrix4x4 to a mesh and returns the result. Note that
/// this does NOT recenter the transformed mesh.
TriMesh TransformMesh(const TriMesh &mesh, const Matrix4f &m);

/// Mirrors a mesh about a plane. Returns the mirrored mesh.
TriMesh MirrorMesh(const TriMesh &mesh, const Plane &plane);

/// Modifies vertices in a mesh if necessary so that the mesh bounds are
/// centered on the origin. Returns the offset vector used to do the
/// recentering.
Vector3f CenterMesh(TriMesh &mesh);

/// Modifies vertices in a mesh if necessary so that they are not shared. Each
/// triangle will have its own 3 unique vertices.
void UnshareMeshVertices(TriMesh &mesh);

/// "Cleans" vertices in a mesh if necessary by rounding values that are close
/// to 0. This helps CGAL not have issues.
void CleanMesh(TriMesh &mesh);

// ----------------------------------------------------------------------------
// Mesh to Ion Shapes and vice-versa.
// ----------------------------------------------------------------------------

/// Converts a TriMesh to an Ion Shape. The alloc_normals and alloc_tex_coords
/// flags indicate whether to allocate space for surface normal and texture
/// coordinate attributes that can be filled in later.
ion::gfx::ShapePtr TriMeshToIonShape(const TriMesh &mesh,
                                     bool alloc_normals = false,
                                     bool alloc_tex_coords = false);

/// Converts a TriMesh to an Ion Shape, replacing what was in the given Shape.
/// The alloc_normals and alloc_tex_coords flags indicate whether to allocate
/// space for surface normal and texture coordinate attributes that can be
/// filled in later.
void UpdateIonShapeFromTriMesh(const TriMesh &mesh, ion::gfx::Shape &shape,
                                     bool alloc_normals = false,
                                     bool alloc_tex_coords = false);

/// Converts an Ion Shape to a TriMesh.
TriMesh IonShapeToTriMesh(const ion::gfx::Shape &shape);

// ----------------------------------------------------------------------------
// I/O.
// ----------------------------------------------------------------------------

/// Writes the given TriMesh in OFF format to the given stream. The description
/// is written as a comment. Note: OFF does not contain normals or texture
/// coords.
void WriteMeshAsOFF(const TriMesh &mesh, const std::string &description,
                    std::ostream &out);

// ----------------------------------------------------------------------------
// Other mesh utilities.
// ----------------------------------------------------------------------------

Bounds ComputeMeshBounds(const TriMesh &mesh);

///@}
