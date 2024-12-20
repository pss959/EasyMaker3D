//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include "Math/Bounds.h"
#include "Math/TriMesh.h"
#include "Math/Types.h"

struct Plane;
struct Ray;

/// \file
/// This file contains functions to aid with intersecting rays with various
/// types.
///
/// \ingroup Math

/// Intersects a Ray with a Bounds. If they intersect at an entry point of the
/// Bounds, this sets distance to the parametric distance to the entry
/// intersection point and returns true. Otherwise, it just returns false.
bool RayBoundsIntersect(const Ray &ray, const Bounds &bounds, float &distance);

/// Version of RayBoundsIntersect() that also returns the Face that was hit and
/// whether the intersection is entering the Bounds or exiting.
bool RayBoundsIntersectFace(const Ray &ray, const Bounds &bounds,
                            float &distance, Bounds::Face &face,
                            bool &is_entry);

/// Intersects a Ray with a Plane. If they intersect, this sets distance to the
/// parametric distance to the intersection point and returns true. Otherwise,
/// it just returns false.
bool RayPlaneIntersect(const Ray &ray, const Plane &plane, float &distance);

/// Intersects a Ray with a triangle formed by 3 points. If an intersection is
/// found, this sets distance to the parametric distance to the intersection
/// point, sets barycentric to the barycentric coordinates of the intersection
/// point, and returns true. Otherwise, it just returns false.
bool RayTriangleIntersect(const Ray &ray, const Point3f &p0,
                          const Point3f &p1, const Point3f &p2,
                          float &distance, Vector3f &barycentric);

/// Intersects a Ray with a TriMesh. If an intersection is found, this sets
/// distance to the parametric distance to the intersection point, fills in the
/// contents of hit with the intersection information, and returns
/// true. Otherwise, it just returns false.
bool RayTriMeshIntersect(const Ray &ray, const TriMesh &mesh,
                         float &distance, TriMesh::Hit &hit);

/// Intersects a Ray with a sphere with the given radius centered at the
/// origin. If it intersects this sets distance to the parametric distance to
/// the intersection point and returns true. Otherwise, it just returns false.
bool RaySphereIntersect(const Ray &ray, float radius, float &distance);

/// Intersects a Ray with an infinite cylinder of the given radius centered at
/// the origin and parallel to the Y axis. If it intersects this sets distance
/// to the parametric distance to the intersection point and returns
/// true. Otherwise, it just returns false.
bool RayCylinderIntersect(const Ray &ray, float radius, float &distance);

/// Intersects a Ray with an infinite cone defined by an apex point, axis
/// vector (from the apex through the center of the cone), and a half angle at
/// the apex. If it intersects this sets distance to the parametric distance to
/// the intersection point and returns true. Otherwise, it just returns false.
bool RayConeIntersect(const Ray &ray, const Point3f &apex,
                      const Vector3f &axis, const Anglef &half_angle,
                      float &distance);

/// Intersects a sphere (given as point and radius) with a Bounds. If they
/// intersect at all, this sets distance to the shortest distance of the sphere
/// center to the Bounds and returns true. Otherwise, it just returns false.
bool SphereBoundsIntersect(const Point3f &center, float radius,
                           const Bounds &bounds, float &distance);

