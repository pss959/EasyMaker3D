#pragma once

#include "Math/Types.h"

/// \file
/// This file contains functions to aid with intersecting rays with various
/// types.
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

/// Intersects a Ray with a unit sphere centered at the origin. If it
/// intersects this sets distance to the parametric distance to the
/// intersection point and returns true. Otherwise, it just returns false.
bool RaySphereIntersect(const Ray &ray, float &distance);

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
