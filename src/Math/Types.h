#pragma once

#include <ostream>
#include <string>
#include <vector>

#include <ion/math/angle.h>
#include <ion/math/angleutils.h>
#include <ion/math/matrix.h>
#include <ion/math/range.h>
#include <ion/math/rotation.h>
#include <ion/math/vector.h>

/// \file
/// This file defines math-related items, including convenience typedefs for
/// math-related Ion objects to make them easier to use inside the application.
///
/// \ingroup Math

typedef ion::math::Anglef    Anglef;
typedef ion::math::Matrix2f  Matrix2f;
typedef ion::math::Matrix3f  Matrix3f;
typedef ion::math::Matrix4f  Matrix4f;
typedef ion::math::Point2f   Point2f;
typedef ion::math::Point2i   Point2i;
typedef ion::math::Point3f   Point3f;
typedef ion::math::Range1f   Range1f;
typedef ion::math::Range2f   Range2f;
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

template <int DIM, typename T> using VectorBase = ion::math::VectorBase<DIM, T>;

typedef ion::math::Range2i Viewport;  ///< Used for viewing operations.

/// Type used for any geometry index.
typedef unsigned int GIndex;

// ----------------------------------------------------------------------------
// Color.
// ----------------------------------------------------------------------------

/// A color represents an RGB or RGBA color. The opacity is 1 by default, but
/// can be overridden.
///
/// \ingroup Math
struct Color : public Vector4f {
    /// Default constructor sets the color to opaque black.
    Color() : Vector4f(0, 0, 0, 1) {}

    /// Constructor from a Vector4f.
    Color(const Vector4f &v) : Vector4f(v) {}

    /// Constructor taking RGB values, setting alpha to 1.
    Color(float r, float g, float b) : Vector4f(r, g, b, 1) {}

    /// Constructor taking RGBA values.
    Color(float r, float g, float b, float a) : Vector4f(r, g, b, a) {}

    /// Returns opaque black.
    static Color Black() { return Color(); }

    /// Returns opaque white.
    static Color White() { return Color(1, 1, 1, 1); }

    /// Returns transparent black (all components 0).
    static Color Clear() { return Color(0, 0, 0, 0); }

    /// Creates a color from hue, saturation, and value.
    static Color FromHSV(float h, float s, float v);

    /// Returns a vector containing hue, saturation, and value for the Color.
    Vector3f ToHSV() const;

    /// Converts the color to a hex string in the format "#RRGGBBAA".
    std::string ToHexString() const;

    /// Parses the given hex string in the format "#RRGGBBAA" or "#RRGGBB",
    /// storing the results in this instance. Returns false on error.
    bool FromHexString(const std::string &str);

    /// Equality operator.
    bool operator==(const Color &c) const = default;

    std::string ToString() const { return ToHexString(); }
};

// ----------------------------------------------------------------------------
// Bounds.
// ----------------------------------------------------------------------------

/// A Bounds struct represents 3D bounds.
///
/// \ingroup Math
struct Bounds : public Range3f {
    /// Faces of bounds, ordered by dimension, then min/max.
    enum class Face { kLeft, kRight, kBottom, kTop, kBack, kFront };

    /// Default constructor creates empty bounds.
    Bounds() : Range3f() {}

    /// (Implicit) constructor to convert from a Range3f.
    Bounds(const Range3f &range) : Range3f(range) {}

    /// Constructor that creates Bounds with min and max points.
    Bounds(const Point3f &min, const Point3f &max) : Range3f(min, max) {}

    /// Constructor that creates Bounds centered on the origin with the given
    /// size.
    Bounds(const Vector3f &size) : Range3f(Point3f::Zero() - .5f * size,
                                           Point3f::Zero() + .5f * size) {}

    /// Returns the center point of the min or max bounds face in the given
    /// dimension.
    Point3f GetFaceCenter(Face face) const;

    /// Returns the dimension for a Face.
    static int GetFaceDim(Face face) { return static_cast<int>(face) / 2; }

    /// Returns true if a Face is on the maximum side of its dimension.
    static int IsFaceMax(Face face)  { return static_cast<int>(face) & 1; }

    /// Returns a Face for the given dimension/is_max pair.
    static Face GetFace(int dim, bool is_max);

    /// Returns the Face that contains the given point, which should be an
    /// intersection point on the bounds. If not, the front face is returned.
    Face GetFaceForPoint(const Point3f &point) const;

    /// Returns the unit normal to a Face.
    static Vector3f GetFaceNormal(Face face) {
        Vector3f normal(0, 0, 0);
        normal[GetFaceDim(face)] = IsFaceMax(face) ? 1.f : -1.f;
        return normal;
    }

    /// Stores all 8 corners of the Bounds in the given array.
    void GetCorners(Point3f corners[8]) const;

    /// Equality operator.
    bool operator==(const Bounds &b) const = default;

    /// Converts to a string to help with debugging. If use_min_max is true,
    /// the min/max values are printed rather than the center and size.
    std::string ToString(bool use_min_max = false) const;
};

// ----------------------------------------------------------------------------
// Plane.
// ----------------------------------------------------------------------------

/// 3D plane.
///
/// \ingroup Math
struct Plane {
    Vector3f normal;    ///< Plane Normal, pointing to positive half-space.
    float    distance;  ///< Signed distance from origin.

    /// The default constructor creates the XY plane.
    Plane();

    /// Constructs from signed distance and normal.
    Plane(float dist, const Vector3f &norm);

    /// Constructs from point and normal.
    Plane(const Point3f &point, const Vector3f &norm);

    /// Constructs from three points.
    Plane(const Point3f &p0, const Point3f &p1, const Point3f &p2);

    /// Returns true if this is the default plane.
    bool IsDefault() const {
        return distance == 0 && normal == Vector3f::AxisZ();
    }

    /// Projects a point onto the plane, returning the result.
    Point3f ProjectPoint(const Point3f &p) const;

    /// Projects a vector onto the plane, returning the result.
    Vector3f ProjectVector(const Vector3f &v) const;

    /// Projects a Rotationf onto the plane, returning the result.
    Rotationf ProjectRotation(const Rotationf &rot) const;

    /// Returns the signed distance of the given point from the plane.
    float GetDistanceToPoint(const Point3f &p) const;

    /// Mirrors a point about the plane, returning the result.
    Point3f MirrorPoint(const Point3f &p) const;

    /// Gets the 4 coefficients of the plane equation (ax + by + cz + d = 0).
    Vector4f GetCoefficients() const;

    /// Equality operator.
    bool operator==(const Plane &p) const = default;

    /// Converts to a string to help with debugging.
    std::string ToString() const;
};

// ----------------------------------------------------------------------------
// Ray.
// ----------------------------------------------------------------------------

/// A Ray struct represents a 3D ray.
///
/// \ingroup Math
struct Ray {
    Point3f  origin;     ///< Origin point of the ray.
    Vector3f direction;  ///< Ray direction, not necessarily normalized.

    /// The default constructor sets the origin to (0,0,0) and the direction to
    /// (0,0,-1);
    Ray() : origin(0, 0, 0), direction(0, 0, -1) {}

    /// Constructor setting both parts.
    Ray(const Point3f &p, const Vector3f &d) : origin(p), direction(d) {}

    /// Returns the point at parametric distance d along the ray.
    Point3f GetPoint(float d) const { return origin + d * direction; }

    /// Converts to a string to help with debugging.
    std::string ToString() const;
};

// ----------------------------------------------------------------------------
// Frustum.
// ----------------------------------------------------------------------------

/// A Frustum struct represents a view frustum used to view a scene. It also
/// contains the Viewport being viewed in for convenience.
///
/// \ingroup Math
struct Frustum {
    Frustum();

    /// Viewport used for the view.
    Viewport  viewport;

    /// Position of the frustum view point. The default is (0,0,10).
    Point3f   position{ 0, 0, 10 };

    ///< Rotation of the frustum from its canonical orientation: looking along
    /// -Z with +Y as the up direction.
    Rotationf orientation;

    /// \name Field of View Angles
    /// These four angles define the field of view. The left and down angles
    /// are typically negative. Note that for VR, the field is not necessarily
    /// symmetric. The default value is -30 degrees for left and down and +30
    /// degrees for right and up.
    ///@{
    Anglef    fov_left;
    Anglef    fov_right;
    Anglef    fov_down;
    Anglef    fov_up;
    ///@}

    /// Distance to near plane from the view point along the view direction.
    /// The default is .01. Note that the name "near" is still reserved by the
    /// compiler on Windows.
    float     pnear;

    /// Distance to far plane from the view point along the view direction. The
    /// default is 20. Note that the name "far" is still reserved by the
    /// compiler on Windows.
    float     pfar;

    /// Convenience that sets the FOV angles to be symmetric based on an FOV
    /// angle and an aspect ratio. The FOV is applied to the larger dimension.
    void SetSymmetricFOV(const Anglef &fov, float aspect);

    /// Convenience that sets the FOV angles from half-angle tangents. This is
    /// what OpenVR supplies.
    void SetFromTangents(float left, float right, float down, float up);

    /// Convenience that returns the main view direction.
    Vector3f GetViewDirection() const {
        return orientation * -Vector3f::AxisZ();
    }

    /// Returns the 2D image rectangle based on the field of view.
    Range2f GetImageRect() const;

    /// Constructs an Ray through the given normalized point on the image
    /// rectangle (in the near plane). (0,0) is the lower-left corner of the
    /// rectangle.
    Ray BuildRay(const Point2f &pt) const;

    /// Projects the given 3D point onto the near plane and returns the result
    /// in normalized coordinates in the image rectangle.
    Point2f ProjectToImageRect(const Point3f &pt) const;

    /// Converts to a string to help with debugging.
    std::string ToString() const;
};

// ----------------------------------------------------------------------------
// TriMesh.
// ----------------------------------------------------------------------------

/// A TriMesh struct represents a 3D triangle mesh.
///
/// \ingroup Math
struct TriMesh {
    /// A point on the mesh resulting from a Ray intersection.
    struct Hit {
        Point3f  point;        ///< Point of intersection.
        Vector3f normal;       ///< Normal to the triangle.
        Vector3i indices;      ///< Indices of triangle the point is on.
        Vector3f barycentric;  ///< Barycentric coordinates at the point.
    };

    /// Vertex points forming the mesh, in no particular order.
    std::vector<Point3f> points;

    /// Point indices forming triangles, 3 per triangle.
    std::vector<GIndex>  indices;

    /// Convenience that returns the number of triangles.
    size_t GetTriangleCount() const { return indices.size() / 3; }

    /// Converts to a (potentially very long) string to help with debugging.
    std::string ToString() const;

    /// Converts to a binary data string for compact output. The format is:
    ///   - *vertex_count*   : uint32
    ///   - *triangle_count* : uint32
    ///   - [*vertices*]     : 3 x float each
    ///   - [*indices*]      : uint32 each
    ///
    /// The endianness of the host computer is used.
    std::string ToBinaryString() const;

    /// Fills in this TriMesh from the contents of a binary string. Returns
    /// false on error.
    bool FromBinaryString(const std::string &str);
};

// ----------------------------------------------------------------------------
// ModelMesh struct.
// ----------------------------------------------------------------------------

/// ModelMesh is a derived TriMesh that adds normals and teture coordinates..
///
/// \ingroup Math
struct ModelMesh : public TriMesh {
    /// Vertex normals
    std::vector<Vector3f> normals;

    /// Vertex texture coordinates.
    std::vector<Point2f>  tex_coords;

    /// Converts to a binary data string for compact output. The format is:
    ///   - *vertex_count*   : uint32
    ///   - *triangle_count* : uint32
    ///   - [*vertices*]     : 3 x float each
    ///   - [*normals*]      : 3 x float each
    ///   - [*tex_coords*]   : 2 x float each
    ///   - [*indices*]      : uint32 each
    ///
    /// The endianness of the host computer is used.
    std::string ToBinaryString() const;

    /// Fills in this ModelMesh from the contents of a binary string. Returns
    /// false on error.
    bool FromBinaryString(const std::string &str);
};

// ----------------------------------------------------------------------------
// Output helpers.
// ----------------------------------------------------------------------------

/// A Stringable is any class that supports ToString().
template <typename T>
concept Stringable = requires(T v)
{
    {v.ToString()} -> std::convertible_to<std::string>;
};


/// Output operator for a Stringable.
template <Stringable T>
inline std::ostream& operator<<(std::ostream& out, const T &t) {
    return out << t.ToString();
}
