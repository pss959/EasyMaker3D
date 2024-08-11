//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include "Math/Types.h"

class Polygon;
class Profile;
struct TriMesh;

/// \file
/// This file defines functions for building TriMesh instances.
///
/// \ingroup Math

/// \name Mesh Building Utilities
/// Each of these functions creates a watertight TriMesh representing some sort
/// of shape.
///@{

/// Builds and returns a watertight TriMesh representing a tetrahedron centered
/// on the origin with all dimensions having the given size and 1 point of the
/// base triangle in front (+Z). Note that this is not a regular tetrahedron.
TriMesh BuildTetrahedronMesh(float size);

/// Builds and returns a watertight TriMesh representing an axis-aligned box
/// centered on the origin with the given sizes in 3 dimensions.
TriMesh BuildBoxMesh(const Vector3f &size);

/// Builds and returns a watertight TriMesh representing a cylinder with the
/// given top and bottom radii, height, and number of sides. The cylinder is
/// centered on the origin and is parallel to the Y axis.
TriMesh BuildCylinderMesh(float top_radius, float bottom_radius,
                          float height, int num_sides);

/// Builds and returns a watertight TriMesh representing a surface of
/// revolution using the given Profile and sweep angle. The Profile is revolved
/// around the Y axis with the base at Y=0. The num_sides parameter is the
/// number of sides if the sweep angle were 360 degrees. Note that a partial
/// mesh (sweep angle < 360 degrees) is *not* necessarily centered on the
/// origin; it is centered on the center of revolution.
TriMesh BuildRevSurfMesh(const Profile &profile, const Anglef &sweep_angle,
                         int num_sides);

/// Builds and returns a watertight TriMesh representing a sphere with the
/// given radius, number of latitudinal rings and longitudinal sectors. The
/// sphere is centered on the origin and has symmetry around the Y axis.
TriMesh BuildSphereMesh(float radius, int num_rings, int num_sectors);

/// Builds and returns a watertight TriMesh representing a torus with the given
/// radii; the outer radius is from the center of the torus to the outer edge
/// of any cross section. The torus is divided into num_sectors sectors around
/// the Y axis. The cross-section of each sector has num_rings sides. The torus
/// is centered on the origin and has symmetry around the Y axis.
TriMesh BuildTorusMesh(float inner_radius, float outer_radius,
                       int num_rings, int num_sectors);

/// Builds and returns a TriMesh representing a tube constructed from a set of
/// connected segments joining the given points, with the given tube diameter
/// and number of sides per segment. If taper is not 1, it is multipled by the
/// diameter to get the ending diameter, with intervening diameters scaled
/// proportionately.
TriMesh BuildTubeMesh(const std::vector<Point3f> &points, float diameter,
                      float taper, int num_sides);

/// Builds and returns a flat TriMesh representing a Polygon. The TriMesh is
/// defined in the Z=0 plane.
TriMesh BuildPolygonMesh(const Polygon &polygon);

/// Extrudes the given 2D Polygon (in the XZ plane) to the given height in Y,
/// returning the resulting TriMesh.
TriMesh BuildExtrudedMesh(const Polygon &polygon, float height);

///@}
