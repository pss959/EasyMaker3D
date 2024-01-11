#pragma once

/// \file
/// This file is included in CGAL-related source files. It defines types that
/// are used throughout the CGAL code.
///
/// \ingroup Math

// Shut up deprecation warnings.
#include <boost/iterator/function_output_iterator.hpp>
#define BOOST_FUNCTION_OUTPUT_ITERATOR_HPP
#define BOOST_BIND_GLOBAL_PLACEHOLDERS

#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Polyhedron_3.h>

struct TriMesh;

// ----------------------------------------------------------------------------
// CGAL types.
// ----------------------------------------------------------------------------

using CKernel     = CGAL::Exact_predicates_exact_constructions_kernel  ;
using CPolyhedron = CGAL::Polyhedron_3<CKernel>;
using CPlane3     = CKernel::Plane_3;
using CPoint2     = CKernel::Point_2;
using CPoint3     = CKernel::Point_3;
using CFI         = CPolyhedron::Facet_const_iterator;
using CHalfedgeDS = CPolyhedron::HalfedgeDS;
using CVertex     = CPolyhedron::HalfedgeDS::Vertex;
using CHFC        = CPolyhedron::Halfedge_around_facet_const_circulator;
using CVI         = CPolyhedron::Vertex_const_iterator;

// ----------------------------------------------------------------------------
// Helper functions.
// ----------------------------------------------------------------------------

/// Builds and returns a CGAL CPolyhedron from a TriMesh.
CPolyhedron TriMeshToCGALPolyhedron(const TriMesh &mesh);

/// Converts a CGAL CPolyhedron to a TriMesh.
TriMesh CGALPolyhedronToTriMesh(const CPolyhedron &poly);
