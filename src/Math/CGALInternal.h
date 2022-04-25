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
#include <CGAL/Nef_polyhedron_3.h>
#include <CGAL/Polygon_2.h>
#include <CGAL/Polygon_mesh_processing/clip.h>
#include <CGAL/Polygon_mesh_processing/self_intersections.h>
#include <CGAL/Polyhedron_3.h>
#include <CGAL/Polyhedron_incremental_builder_3.h>
#include <CGAL/convex_hull_3.h>
#include <CGAL/minkowski_sum_3.h>

// ----------------------------------------------------------------------------
// CGAL types.
// ----------------------------------------------------------------------------

typedef CGAL::Exact_predicates_exact_constructions_kernel   CKernel;
typedef CGAL::Nef_polyhedron_3<CKernel>                     CNefPolyhedron;
typedef CGAL::Polyhedron_3<CKernel>                         CPolyhedron;
typedef CKernel::Plane_3                                    CPlane3;
typedef CKernel::Point_2                                    CPoint2;
typedef CKernel::Point_3                                    CPoint3;
typedef CPolyhedron::Facet_const_iterator                   CFI;
typedef CPolyhedron::HalfedgeDS                             CHalfedgeDS;
typedef CPolyhedron::HalfedgeDS::Vertex                     CVertex;
typedef CPolyhedron::Halfedge_around_facet_const_circulator CHFC;
typedef CPolyhedron::Vertex_const_iterator                  CVI;

// ----------------------------------------------------------------------------
// Helper functions.
// ----------------------------------------------------------------------------

struct TriMesh;

/// Builds and returns a CGAL CPolyhedron from a TriMesh.
CPolyhedron BuildCGALPolyhedron(const TriMesh &mesh);
