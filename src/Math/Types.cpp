#include "Math/Types.h"

#include <cctype>
#include <cmath>

#include <ion/math/angleutils.h>
#include <ion/math/vectorutils.h>

#include "Math/Linear.h"
#include "Util/Assert.h"
#include "Util/String.h"

// ----------------------------------------------------------------------------
// Helper functions.
// ----------------------------------------------------------------------------

namespace {

/// Overrides the standard Ion string printing version to be more compact.
template <typename T> std::string ToString_(const T &t) {
    ASSERTM(false, "Unspecialized ToString_() called!");
    return "";
}
template <> std::string ToString_(const Point3f &t) {
    return "[" +
        Util::ToString(t[0], .001f) + " " +
        Util::ToString(t[1], .001f) + " " +
        Util::ToString(t[2], .001f) + "]";
}
template <> std::string ToString_(const Vector3f &t) {
    return "[" +
        Util::ToString(t[0], .001f) + " " +
        Util::ToString(t[1], .001f) + " " +
        Util::ToString(t[2], .001f) + "]";
}

}  // anonymous namespace

// ----------------------------------------------------------------------------
// Color functions.
// ----------------------------------------------------------------------------

Color Color::FromHSV(float h, float s, float v) {
    const float hs = (h == 1.f ? 0.f : 6.f * h);
    const int   hue_sextant = static_cast<int>(floorf(hs));
    const float hue_frac = hs - hue_sextant;

    const float t1 = v * (1.f - s);
    const float t2 = v * (1.f - (s * hue_frac));
    const float t3 = v * (1.f - (s * (1.f - hue_frac)));

    switch (hue_sextant) {
      case 0:
        return Color(v, t3, t1);
      case 1:
        return Color(t2, v, t1);
      case 2:
        return Color(t1, v, t3);
      case 3:
        return Color(t1, t2, v);
      case 4:
        return Color(t3, t1, v);
      default:
        return Color(v, t1, t2);
    }

}

Vector3f Color::ToHSV() const {
    const float r = (*this)[0];
    const float g = (*this)[1];
    const float b = (*this)[2];

    const float max = std::max(r, std::max(g, b));
    const float min = std::min(r, std::min(g, b));
    const float diff = max - min;

    const float val = max;
    const float sat = (max > 0 ? diff / max : 0);

    float hue = 0;
    if (sat > 0) {
	if (r == max)
	    hue = (g - b) / diff;
	else if (g == max)
	    hue = 2 + (b - r) / diff;
	else
	    hue = 4 + (r - g) / diff;
        if (hue < 0)
            hue += 6;
	hue /= 6;
    }

    return Vector3f(hue, sat, val);
}

bool Color::FromHexString(const std::string &str) {
    if (str[0] != '#' || (str.size() != 7U && str.size() != 9U))
        return false;

    for (size_t i = 1; i < str.size(); ++i)
        if (! std::isxdigit(str[i]))
            return false;

    uint32_t n;
    std::istringstream(&str[1]) >> std::hex >> n;

    if (str.size() == 9) {  // #RRGGBBAA format.
        Set(static_cast<float>((n >> 24) & 0xff) / 255.f,
            static_cast<float>((n >> 16) & 0xff) / 255.f,
            static_cast<float>((n >>  8) & 0xff) / 255.f,
            static_cast<float>( n        & 0xff) / 255.f);
    }
    else if (str.size() == 7) {  // #RRGGBB format.
        Set(static_cast<float>((n >> 16) & 0xff) / 255.f,
            static_cast<float>((n >>  8) & 0xff) / 255.f,
            static_cast<float>( n        & 0xff) / 255.f,
            1.f);
    }
    return true;
}

// ----------------------------------------------------------------------------
// Bounds functions.
// ----------------------------------------------------------------------------

Bounds::Face Bounds::GetFace(int dim, bool is_max) {
    // Face enum values are ordered to make this work.
    ASSERT(dim >= 0 && dim <= 2);
    return static_cast<Face>(2 * dim + (is_max ? 1 : 0));
}

Bounds::Face Bounds::GetFaceForPoint(const Point3f &point) const {
    const auto &min_pt = GetMinPoint();
    const auto &max_pt = GetMaxPoint();
    for (int dim = 0; dim < 3; ++dim) {
        if (AreClose(point[dim], min_pt[dim]))
            return GetFace(dim, false);
        if (AreClose(point[dim], max_pt[dim]))
            return GetFace(dim, true);
    }
    // Just in case of failure.
    return Face::kFront;
}

void Bounds::GetCorners(Point3f corners[8]) const {
    const auto &min_pt = GetMinPoint();
    const auto &max_pt = GetMaxPoint();
    corners[0].Set(min_pt[0], min_pt[1], min_pt[0]);
    corners[1].Set(min_pt[0], min_pt[1], max_pt[2]);
    corners[2].Set(min_pt[0], max_pt[1], min_pt[2]);
    corners[3].Set(min_pt[0], max_pt[1], max_pt[2]);
    corners[4].Set(max_pt[0], min_pt[1], min_pt[2]);
    corners[5].Set(max_pt[0], min_pt[1], max_pt[2]);
    corners[6].Set(max_pt[0], max_pt[1], min_pt[2]);
    corners[7].Set(max_pt[0], max_pt[1], max_pt[2]);
}

std::string Bounds::ToString(bool use_min_max) const {
    if (use_min_max)
        return ("B ["  + Util::ToString(GetMinPoint()) +
                " => " + Util::ToString(GetMaxPoint()) + "]");
    else
        return ("B [c="  + Util::ToString(GetCenter())  +
                " s="    + Util::ToString(GetSize())    + "]");
}

// ----------------------------------------------------------------------------
// Plane functions.
// ----------------------------------------------------------------------------

Plane::Plane(const Point3f &point, const Vector3f &norm) {
    normal   = ion::math::Normalized(norm);
    distance = ion::math::Dot(normal, point - Point3f::Zero());
}

Plane::Plane(const Point3f &p0, const Point3f &p1, const Point3f &p2) {
    normal   = ion::math::Normalized(ion::math::Cross(p1 - p0, p2 - p0));
    distance = ion::math::Dot(normal, Vector3f(p0));
}

Point3f Plane::ProjectPoint(const Point3f &p) const {
    return p - GetDistanceToPoint(p) * normal;
}

Vector3f Plane::ProjectVector(const Vector3f &v) const {
    return v - ion::math::Dot(normal, v) * normal;
}

Rotationf Plane::ProjectRotation(const Rotationf &rot) const {
    // Find the component of the plane normal with the smallest absolute
    // value. Use that to choose an axis that is guaranteed not to be close to
    // the normal.
    const Vector3f axis = GetAxis(GetMinAbsElementIndex(normal));

    // Project the axis and rotated axis onto the plane, then compute the
    // rotation between them. This is the result.
    return Rotationf::RotateInto(ProjectVector(axis),
                                 ProjectVector(rot * axis));
}

float Plane::GetDistanceToPoint(const Point3f &p) const {
    return ion::math::Dot(normal, Vector3f(p)) - distance;
}

Point3f Plane::MirrorPoint(const Point3f &p) const {
    return p - 2.f * GetDistanceToPoint(p) * normal;
}

Vector4f Plane::GetCoefficients() const {
    return Vector4f(normal[0], normal[1], normal[2], distance);
}

std::string Plane::ToString() const {
    return ("PL [n="  + ToString_(normal)  +
            " d="     + Util::ToString(distance) +
            "]");
}

// ----------------------------------------------------------------------------
// Ray functions.
// ----------------------------------------------------------------------------

std::string Ray::ToString() const {
    return ("RAY [o="  + ToString_(origin)  +
            " d="      + ToString_(direction) +
            "]");
}

// ----------------------------------------------------------------------------
// Frustum functions.
// ----------------------------------------------------------------------------

void Frustum::SetSymmetricFOV(const Anglef &fov, float aspect) {
    // Use the specified FOV for the larger dimension and compute the FOV for
    // the other dimension.
    using ion::math::ArcTangent;
    using ion::math::Tangent;
    Anglef half_v_fov, half_h_fov;
    if (aspect >= 1.f) {
        // Wider than tall; use given FOV in horizontal dimension.
        half_h_fov = .5f * fov;
        half_v_fov = ArcTangent(Tangent(half_h_fov) / aspect);
    }
    else {
        // Taller than wide; use given FOV in vertical dimension.
        half_v_fov = .5f * fov;
        half_h_fov = ArcTangent(Tangent(half_v_fov) * aspect);
    }

    fov_left    = -half_h_fov;
    fov_right   =  half_h_fov;
    fov_down    = -half_v_fov;
    fov_up      =  half_v_fov;
}

Ray Frustum::BuildRay(const Point2f &pt) const {
    // Ignore position and orientation for now; assume the direction is -Z. Use
    // the FOV angles to get the lower-left and upper-right corners of the
    // image rectangle in the near plane.  All values follow this form:
    // tan(-fov_left) = left_x / near => left_x = near * tan(-fov_left).
    Point2f ll(pnear * ion::math::Tangent(fov_left),
               pnear * ion::math::Tangent(fov_down));
    Point2f ur(pnear * ion::math::Tangent(fov_right),
               pnear * ion::math::Tangent(fov_up));
    Point2f rect_pt = ll + Vector2f(pt) * (ur - ll);

    // Use this to compute the point in the canonical image plane.  This point
    // gives us the unrotated direction relative to the view position. The
    // length of this vector is the distance from the eyepoint to the ray
    // origin;
    Vector3f direction(rect_pt[0], rect_pt[1], -pnear);
    const float distance = ion::math::Length(direction);

    // Rotate the normalized direction by the orientation.
    direction = orientation * ion::math::Normalized(direction);

    // Use the distance and the correct direction to get the ray origin.
    return Ray(position + distance * direction, direction);
}

std::string Frustum::ToString() const {
    return ("FR [vp="  + Util::ToString(viewport) +
            " p="      + ToString_(position) +
            " o="      + Util::ToString(orientation) +
            " fov=(l:" + Util::ToString(fov_left) +
            " r:"      + Util::ToString(fov_right) +
            " u:,"     + Util::ToString(fov_up) +
            " d:,"     + Util::ToString(fov_down) +
            ") n="     + Util::ToString(pnear) +
            " f="      + Util::ToString(pfar) +
            "]");
}

// ----------------------------------------------------------------------------
// TriMesh functions.
// ----------------------------------------------------------------------------

std::string TriMesh::ToString() const {
    const size_t pc = points.size();
    const size_t tc = GetTriangleCount();
    std::string s = "TriMesh with " + Util::ToString(pc) + " points and " +
        Util::ToString(tc) + " triangles:\n";
    for (size_t i = 0; i < pc; ++i)
        s += "   [" + Util::ToString(i) + "] " + ToString_(points[i]) + "\n";

    auto i2s = [&](size_t index){ return Util::ToString(indices[index]); };
    for (size_t i = 0; i < tc; ++i)
        s += "   TRI [" +
            i2s(3 * i + 0) + " " +
            i2s(3 * i + 1) + " " +
            i2s(3 * i + 2) + "]\n";
    return s;
}
