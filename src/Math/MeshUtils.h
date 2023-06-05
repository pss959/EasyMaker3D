#pragma once

#include <ostream>
#include <string>

#include <ion/gfx/shape.h>

#include "Math/Types.h"

struct SlicedMesh;
struct Spin;
struct Taper;

/// \file
/// Mesh Utility functions
///
/// \ingroup Math

// ----------------------------------------------------------------------------
// Mesh transformations.
// ----------------------------------------------------------------------------

/// \name General transformations.
///@{

/// Applies the given scale to a mesh and returns the result.
TriMesh ScaleMesh(const TriMesh &mesh, const Vector3f &scale);

/// Applies the given rotation to a mesh and returns the result.
TriMesh RotateMesh(const TriMesh &mesh, const Rotationf &rot);

/// Applies the given Matrix4x4 to a mesh and returns the result. Note that
/// this does NOT recenter the transformed mesh.
TriMesh TransformMesh(const TriMesh &mesh, const Matrix4f &m);

///@}

/// \name Deformations.
///@{

/// Bends a SlicedMesh using the given Spin struct.
TriMesh BendMesh(const SlicedMesh &sliced_mesh, const Spin &spin);

/// Mirrors a mesh about a plane. Returns the mirrored mesh.
TriMesh MirrorMesh(const TriMesh &mesh, const Plane &plane);

/// Tapers a SlicedMesh using the given Taper struct.
TriMesh TaperMesh(const SlicedMesh &sliced_mesh, const Taper &taper);

/// Twists a SlicedMesh using the given Spin struct.
TriMesh TwistMesh(const SlicedMesh &sliced_mesh, const Spin &spin);

///@}

/// \name Other modifications.
///@{

/// Returns a new version of a mesh with the bounds centered on the origin.
TriMesh CenterMesh(const TriMesh &mesh);

/// Modifies vertices in a mesh if necessary so that they are not shared. Each
/// triangle will have its own 3 unique vertices.
void UnshareMeshVertices(TriMesh &mesh);

/// If there any triangles that share the same vertices, this removes them.
/// This can be useful when an operation such as BendMesh() can result in faces
/// that abut exactly.
void RemoveDualTriangles(TriMesh &mesh);

/// "Cleans" vertices in a mesh if necessary by rounding values that are close
/// to 0. Also removes degenerate triangles and unused vertex points. This
/// helps CGAL not have issues.
void CleanMesh(TriMesh &mesh);

///@}

// ----------------------------------------------------------------------------
// Mesh to Ion Shapes and vice-versa.
// ----------------------------------------------------------------------------

/// \name Conversion to and from Ion Shape instances.
///@{

/// Converts a TriMesh to an Ion Shape. The alloc_normals and alloc_tex_coords
/// flags indicate whether to allocate space for surface normal and texture
/// coordinate attributes that can be filled in later.
ion::gfx::ShapePtr TriMeshToIonShape(const TriMesh &mesh,
                                     bool alloc_normals = false,
                                     bool alloc_tex_coords = false,
                                     bool is_dynamic = false);

/// Converts a TriMesh to an Ion Shape, replacing what was in the given Shape.
/// The alloc_normals and alloc_tex_coords flags indicate whether to allocate
/// space for surface normal and texture coordinate attributes that can be
/// filled in later.
void UpdateIonShapeFromTriMesh(const TriMesh &mesh, ion::gfx::Shape &shape,
                               bool alloc_normals = false,
                               bool alloc_tex_coords = false,
                               bool is_dynamic = false);

/// Converts an Ion Shape to a TriMesh.
TriMesh IonShapeToTriMesh(const ion::gfx::Shape &shape);

///@}

// ----------------------------------------------------------------------------
// I/O.
// ----------------------------------------------------------------------------

/// \name I/O.
///@{

/// Writes the given TriMesh in OFF format to the given stream. The description
/// is written as a comment. Note: OFF does not contain normals or texture
/// coords.
void WriteMeshAsOFF(const TriMesh &mesh, const std::string &description,
                    std::ostream &out);

///@}

// ----------------------------------------------------------------------------
// Other mesh utilities.
// ----------------------------------------------------------------------------

/// \name Other utilities.
///@{

Bounds ComputeMeshBounds(const TriMesh &mesh);

///@}
