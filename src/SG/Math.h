#pragma once

#include <string>
#include <vector>

#include <ion/math/angle.h>
#include <ion/math/matrix.h>
#include <ion/math/range.h>
#include <ion/math/rotation.h>
#include <ion/math/vector.h>

#include <Assert.h>

namespace SG {

//! \file
//! This file contains math-related items. It also defines convenience typedefs
//! for math-related Ion objects to make them more useful inside SG classes.

typedef ion::math::Anglef    Anglef;
typedef ion::math::Matrix2f  Matrix2f;
typedef ion::math::Matrix3f  Matrix3f;
typedef ion::math::Matrix4f  Matrix4f;
typedef ion::math::Point2f   Point2f;
typedef ion::math::Point2i   Point2i;
typedef ion::math::Point3f   Point3f;
typedef ion::math::Range2i   Range2i;
typedef ion::math::Range3f   Range3f;
typedef ion::math::Rotationf Rotationf;
typedef ion::math::Vector2f  Vector2f;
typedef ion::math::Vector2i  Vector2i;
typedef ion::math::Vector2ui Vector2ui;
typedef ion::math::Vector3f  Vector3f;
typedef ion::math::Vector3i  Vector3i;
typedef ion::math::Vector3ui Vector3ui;
typedef ion::math::Vector4f  Vector4f;
typedef ion::math::Vector4i  Vector4i;
typedef ion::math::Vector4ui Vector4ui;

//! A Bounds struct represents 3D bounds.
struct Bounds : public Range3f {
    //! Faces of bounds, ordered by dimension, then min/max.
    enum class Face { kLeft, kRight, kBottom, kTop, kBack, kFront };

    //! Default constructor creates empty bounds.
    Bounds() : Range3f() {}

    //! Constructor that creates Bounds centered on the origin with the given
    //! size.
    Bounds(const Vector3f size) : Range3f(Point3f::Zero() - .5f * size,
                                          Point3f::Zero() + .5f * size) {}

    //! Returns the dimension for a Face.
    static int GetFaceDim(Face face) { return static_cast<int>(face) / 2; }

    //! Returns true if a Face is on the maximum side of its dimension.
    static int IsFaceMax(Face face)  { return static_cast<int>(face) & 1; }

    //! Returns a Face for the given dimension/is_max pair.
    static Face GetFace(int dim, bool is_max) {
        // Face enum values are ordered to make this work.
        ASSERT(dim >= 0 && dim <= 2);
        return static_cast<Face>(2 * dim + (is_max ? 1 : 0));
    }

    //! Returns the unit normal to a Face.
    static Vector3f GetFaceNormal(Face face) {
        Vector3f normal(0, 0, 0);
        normal[GetFaceDim(face)] = IsFaceMax(face) ? 1.f : -1.f;
        return normal;
    }
};

//! 3D plane.
struct Plane {
    float    distance;  //!< Distance from origin.
    Vector3f normal;    //!< Plane Normal, pointing to positive half-space.

    //! The default constructor creates the XY plane.
    Plane() : distance(0.f), normal(Vector3f::AxisZ()) {}

    //! Constructs from distance and normal.
    Plane(float dist, const Vector3f &norm) : distance(dist), normal(norm) {}

    //! Constructs from point and normal.
    Plane(const Point3f &point, const Vector3f &norm);

    //! Constructs from three points.
    Plane(const Point3f &p0, const Point3f &p1, const Point3f &p2);

    //! Converts to a string to help with debugging.
    std::string ToString() const;
};

//! A Ray struct represents a 3D ray.
struct Ray {
    Point3f  origin;     //!< Origin point of the ray.
    Vector3f direction;  //!< Ray direction, not necessarily normalized.

    //! The default constructor sets the origin to (0,0,0) and the direction to
    //! (0,0,-1);
    Ray() : origin(0, 0, 0), direction(0, 0, -1) {}

    //! Constructor setting both parts.
    Ray(const Point3f &p, const Vector3f &d) : origin(p), direction(d) {}

    //! Returns the point at parametric distance d along the ray.
    Point3f GetPoint(float d) const { return origin + d * direction; }

    //! Converts to a string to help with debugging.
    std::string ToString() const;
};

//! A TriMesh struct represents a 3D triangle mesh.
struct TriMesh {
    //! A point on the mesh resulting from a Ray intersection.
    struct Hit {
        Point3f  point;        //!< Point of intersection.
        Vector3f normal;       //!< Normal to the triangle.
        Vector3i indices;      //!< Indices of triangle the point is on.
        Vector3f barycentric;  //!< Barycentric coordinates at the point.
    };

    //! Vertex points forming the mesh, in no particular order.
    std::vector<Point3f>  points;

    //! Surface normals at the points, in the same order.
    // XXXX Not needed? std::vector<Vector3f> normals;

    //! Point indices forming triangles, 3 per triangle.
    std::vector<int>      indices;
};

// ----------------------------------------------------------------------------
// Transformation functions.
// ----------------------------------------------------------------------------

//! Transforms a Ray by a matrix. This does not normalize the resulting
//! direction vector, so that parametric distances are preserved.
Ray TransformRay(const Ray &ray, const Matrix4f &m);

//! Transforms a Bounds by a matrix, returning a new (aligned) Bounds that
//! encloses the result.
Bounds TransformBounds(const Bounds &bounds, const Matrix4f &m);

// ----------------------------------------------------------------------------
// Intersection functions.
// ----------------------------------------------------------------------------

//! Intersects a Ray with a Bounds. If they intersect at an entry point of the
//! Bounds, this sets distance to the parametric distance to the entry
//! intersection point and returns true. Otherwise, it just returns false.
bool RayBoundsIntersect(const Ray &ray, const Bounds &bounds, float &distance);

//! Version of RayBoundsIntersect() that also returns the Face that was hit and
//! whether the intersection is entering the Bounds or exiting.
bool RayBoundsIntersectFace(const Ray &ray, const Bounds &bounds,
                            float &distance, Bounds::Face &face,
                            bool &is_entry);

//! Intersects a Ray with a Plane. If they intersect, this sets distance to the
//! parametric distance to the intersection point and returns true. Otherwise,
//! it just returns false.
bool RayPlaneIntersect(const Ray &ray, const Plane &plane, float &distance);

//! Intersects a Ray with a triangle formed by 3 points. If an intersection is
//! found, this sets distance to the parametric distance to the intersection
//! point, sets barycentric to the barycentric coordinates of the intersection
//! point, and returns true. Otherwise, it just returns false.
bool RayTriangleIntersect(const Ray &ray, const Point3f &p0,
                          const Point3f &p1, const Point3f &p2,
                          float &distance, Vector3f &barycentric);

//! Intersects a Ray with a TriMesh. If an intersection is found, this sets
//! distance to the parametric distance to the intersection point, fills in the
//! contents of hit with the intersection information, and returns
//! true. Otherwise, it just returns false.
bool RayTriMeshIntersect(const Ray &ray, const TriMesh &mesh,
                         float &distance, TriMesh::Hit &hit);

// ----------------------------------------------------------------------------
// General linear algebra functions.
// ----------------------------------------------------------------------------

//! Returns true if two values are close enough to each other within a
// tolerance.
bool AreClose(float a, float b, float tolerance = .0001f);

//! Returns true if two vectors are close enough to each other within a
// tolerance, which is the square of the distance.
bool AreClose(const Vector3f &a, const Vector3f &b, float tolerance = .0001f);

//! Returns the index of the minimum element of a Vector3f.
int GetMinElementIndex(const Vector3f &v);

//! Returns the index of the maximum element of a Vector3f.
int GetMaxElementIndex(const Vector3f &v);

//! Returns the index of the minimum element (by absolute value) of a Vector3f.
int GetMinAbsElementIndex(const Vector3f &v);

//! Returns the index of the maximum element (by absolute value) of a Vector3f.
int GetMaxAbsElementIndex(const Vector3f &v);

//! Computes the normal to a triangle defined by three points.
Vector3f ComputeNormal(const Point3f &p0, const Point3f &p1, const Point3f &p2);

//! Computes and returns barycentric coordinates for point p with respect to
//! triangle ABC. Returns false if the point is outside the triangle.
bool ComputeBarycentric(const Point2f &p, const Point2f & a,
                        const Point2f &b, const Point2f &c, Vector3f &bary);

}  // namespace SG
