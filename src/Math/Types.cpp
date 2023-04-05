#include "Math/Types.h"

#include <cctype>
#include <cmath>
#include <iomanip>
#include <sstream>

#include <ion/base/bufferbuilder.h>
#include <ion/math/angleutils.h>
#include <ion/math/transformutils.h>
#include <ion/math/vectorutils.h>

#include "Math/Linear.h"
#include "Math/ToString.h"
#include "Util/Assert.h"
#include "Util/String.h"
#include "Util/Tuning.h"

// ----------------------------------------------------------------------------
// Helper functions.
// ----------------------------------------------------------------------------

namespace {

/// Overrides the standard Ion string printing version to be more compact and
/// to round to a reasonable precision.
template <typename T> std::string ToString_(const T &t) {
    return Util::ToString(t);
}
template <> std::string ToString_(const float &f) {
    return Math::ToString(f, .001f);
}
template <> std::string ToString_(const Point3f &t) {
    return Math::ToString(t, .001f);
}
template <> std::string ToString_(const Vector3f &t) {
    return Math::ToString(t, .001f);
}
template <> std::string ToString_(const Anglef &a) {
    return Math::ToString(a.Degrees(), .1f);
}
template <> std::string ToString_(const Rotationf &r) {
    Vector3f axis;
    Anglef   angle;
    r.GetAxisAndAngle(&axis, &angle);
    return "R[" + ToString_(axis) + ", " + ToString_(angle) + "]";
}

/// This is used by TriMesh::FromBinaryString() to parse binary data.
template <typename T> T ParseBinary_(const uint8 *&bp) {
    const T val = *reinterpret_cast<const T *>(bp);
    bp += sizeof(val);
    return val;
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

std::string Color::ToHexString() const {
    auto to_hex = [](float f){
        std::ostringstream out;
        out << std::hex << std::setfill('0') << std::setw(2)
            << std::lround(255 * f);
        return out.str();
    };
    const auto &v = *this;
    return "#" + to_hex(v[0]) + to_hex(v[1]) + to_hex(v[2]) + to_hex(v[3]);
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

Point3f Bounds::GetFaceCenter(Face face) const {
    Point3f center = GetCenter();
    const int dim = GetFaceDim(face);
    center[dim] = IsFaceMax(face) ? GetMaxPoint()[dim] : GetMinPoint()[dim];
    return center;
}

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
    corners[0].Set(min_pt[0], min_pt[1], min_pt[2]);
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
        return ("B["   + ToString_(GetMinPoint()) +
                " => " + ToString_(GetMaxPoint()) + "]");
    else
        return ("B[c="  + ToString_(GetCenter())  +
                " s="   + ToString_(GetSize())    + "]");
}

// ----------------------------------------------------------------------------
// Plane functions.
// ----------------------------------------------------------------------------

Plane::Plane(const Point3f &point, const Vector3f &norm) {
    normal   = ion::math::Normalized(norm);
    distance = SignedDistance(point, normal);
}

Plane::Plane(const Point3f &p0, const Point3f &p1, const Point3f &p2) {
    normal   = ion::math::Normalized(ion::math::Cross(p1 - p0, p2 - p0));
    distance = SignedDistance(p0, normal);
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
    const Vector3f perp_axis = GetAxis(GetMinAbsElementIndex(normal));

    // Project the axis and rotated axis onto the plane, then compute the
    // rotation between them. This is the result.
    return Rotationf::RotateInto(ProjectVector(perp_axis),
                                 ProjectVector(rot * perp_axis));
}

float Plane::GetDistanceToPoint(const Point3f &p) const {
    return SignedDistance(p, normal) - distance;
}

Point3f Plane::MirrorPoint(const Point3f &p) const {
    return p - 2.f * GetDistanceToPoint(p) * normal;
}

Vector4f Plane::GetCoefficients() const {
    return Vector4f(normal[0], normal[1], normal[2], -distance);
}

std::string Plane::ToString() const {
    return "PL [n=" + ToString_(normal) + " d=" + ToString_(distance) + "]";
}

// ----------------------------------------------------------------------------
// Ray functions.
// ----------------------------------------------------------------------------

std::string Ray::ToString() const {
    return "RAY [o=" + ToString_(origin) + " d=" + ToString_(direction) + "]";
}

// ----------------------------------------------------------------------------
// Frustum functions.
// ----------------------------------------------------------------------------

Frustum::Frustum() {
    const Anglef fov_angle = Anglef::FromDegrees(TK::kFOVHalfAngle);
    fov_left  = fov_down  = -fov_angle;
    fov_right = fov_up    =  fov_angle;

    pnear = TK::kNearDistance;
    pfar  = TK::kFarDistance;
}

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

void Frustum::SetFromTangents(float left, float right, float down, float up) {
    using ion::math::ArcTangent;
    fov_left  = ArcTangent(left);
    fov_right = ArcTangent(right);
    fov_down  = ArcTangent(down);
    fov_up    = ArcTangent(up);
}

Range2f Frustum::GetImageRect() const {
    // Use the FOV angles to get the lower-left and upper-right corners of the
    // image rectangle in the near plane.  All values follow this form:
    // tan(-fov_left) = left_x / near => left_x = near * tan(-fov_left).
    const Point2f ll(pnear * ion::math::Tangent(fov_left),
                     pnear * ion::math::Tangent(fov_down));
    const Point2f ur(pnear * ion::math::Tangent(fov_right),
                     pnear * ion::math::Tangent(fov_up));
    return Range2f(ll, ur);
}

Ray Frustum::BuildRay(const Point2f &pt) const {
    const Range2f image_rect = GetImageRect();
    const Point2f rect_pt =
        image_rect.GetMinPoint() + Vector2f(pt) * image_rect.GetSize();

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

Point2f Frustum::ProjectToImageRect(const Point3f &pt) const {
    // Apply the projection and view matrices and convert to normalized image
    // rectangle coordinates.
    const Matrix4f m = GetProjectionMatrix(*this) * GetViewMatrix(*this);
    const Point3f  p = ion::math::ProjectPoint(m, pt);
    return Point2f(.5f + .5f * p[0],
                   .5f + .5f * p[1]);
}

std::string Frustum::ToString() const {
    return ("FR [vp="  + ToString_(viewport) +
            " p="      + ToString_(position) +
            " o="      + ToString_(orientation) +
            " fov=(l:" + ToString_(fov_left) +
            " r:"      + ToString_(fov_right) +
            " u:"      + ToString_(fov_up) +
            " d:"      + ToString_(fov_down) +
            ") n="     + ToString_(pnear) +
            " f="      + ToString_(pfar) +
            "]");
}

// ----------------------------------------------------------------------------
// TriMesh functions.
// ----------------------------------------------------------------------------

std::string TriMesh::ToString() const {
    const size_t pc = points.size();
    const size_t tc = GetTriangleCount();
    std::string s = "TriMesh with " + ToString_(pc) + " points and " +
        ToString_(tc) + " triangles:\n";
    for (size_t i = 0; i < pc; ++i)
        s += "   [" + ToString_(i) + "] " + ToString_(points[i]) + "\n";

    auto i2s = [&](size_t index){ return ToString_(indices[index]); };
    for (size_t i = 0; i < tc; ++i)
        s += "   TRI [" +
            i2s(3 * i + 0) + " " +
            i2s(3 * i + 1) + " " +
            i2s(3 * i + 2) + "]\n";
    return s;
}

std::string TriMesh::ToBinaryString() const {
    const size_t pc = points.size();
    const size_t tc = GetTriangleCount();

    ion::base::BufferBuilder bb;
    bb.Append(static_cast<uint32>(pc));
    bb.Append(static_cast<uint32>(tc));
    bb.AppendArray(&points[0],  points.size());
    bb.AppendArray(&indices[0], indices.size());
    return bb.Build();
}

bool TriMesh::FromBinaryString(const std::string &str) {
    points.clear();
    indices.clear();

    const uint8 *bp = reinterpret_cast<const uint8 *>(str.c_str());

    // Point and triangle counts.
    size_t bytes_left = str.size();
    uint32 pc, tc;
    if (bytes_left < 2 * sizeof(pc))
        return false;
    pc = ParseBinary_<uint32>(bp);
    tc = ParseBinary_<uint32>(bp);
    bytes_left -= 2 * sizeof(pc);

    points.reserve(pc);
    indices.reserve(3 * tc);

    // Points.
    if (bytes_left < pc * 3 * sizeof(float))
        return false;
    for (size_t i = 0; i < pc; ++i) {
        const float x = ParseBinary_<float>(bp);
        const float y = ParseBinary_<float>(bp);
        const float z = ParseBinary_<float>(bp);
        points.push_back(Point3f(x, y, z));
    }
    bytes_left -= pc * 3 * sizeof(float);

    // Indices.
    if (bytes_left < tc * 3 * sizeof(GIndex))
        return false;
    for (size_t i = 0; i < 3 * tc; ++i)
        indices.push_back(ParseBinary_<GIndex>(bp));

    return true;
}

// ----------------------------------------------------------------------------
// ModelMesh functions.
// ----------------------------------------------------------------------------

std::string ModelMesh::ToBinaryString() const {
    const size_t pc = points.size();
    const size_t tc = GetTriangleCount();

    ASSERT(normals.size()    == pc);
    ASSERT(tex_coords.size() == pc);

    ion::base::BufferBuilder bb;
    bb.Append(static_cast<uint32>(pc));
    bb.Append(static_cast<uint32>(tc));
    bb.AppendArray(&points[0],     points.size());
    bb.AppendArray(&normals[0],    normals.size());
    bb.AppendArray(&tex_coords[0], tex_coords.size());
    bb.AppendArray(&indices[0],    indices.size());
    return bb.Build();
}

bool ModelMesh::FromBinaryString(const std::string &str) {
    points.clear();
    indices.clear();
    normals.clear();
    tex_coords.clear();

    const uint8 *bp = reinterpret_cast<const uint8 *>(str.c_str());

    // Point and triangle counts.
    size_t bytes_left = str.size();
    uint32 pc, tc;
    if (bytes_left < 2 * sizeof(pc))
        return false;
    pc = ParseBinary_<uint32>(bp);
    tc = ParseBinary_<uint32>(bp);
    bytes_left -= 2 * sizeof(pc);

    points.reserve(pc);
    indices.reserve(3 * tc);

    // Points.
    if (bytes_left < pc * 3 * sizeof(float))
        return false;
    for (size_t i = 0; i < pc; ++i) {
        const float x = ParseBinary_<float>(bp);
        const float y = ParseBinary_<float>(bp);
        const float z = ParseBinary_<float>(bp);
        points.push_back(Point3f(x, y, z));
    }
    bytes_left -= pc * 3 * sizeof(float);

    // Normals.
    if (bytes_left < pc * 3 * sizeof(float))
        return false;
    for (size_t i = 0; i < pc; ++i) {
        const float nx = ParseBinary_<float>(bp);
        const float ny = ParseBinary_<float>(bp);
        const float nz = ParseBinary_<float>(bp);
        normals.push_back(Vector3f(nx, ny, nz));
    }
    bytes_left -= pc * 3 * sizeof(float);

    // Texture coordinates.
    if (bytes_left < pc * 2 * sizeof(float))
        return false;
    for (size_t i = 0; i < pc; ++i) {
        const float u = ParseBinary_<float>(bp);
        const float v = ParseBinary_<float>(bp);
        tex_coords.push_back(Point2f(u, v));
    }
    bytes_left -= pc * 2 * sizeof(float);

    // Indices.
    if (bytes_left < tc * 3 * sizeof(GIndex))
        return false;
    for (size_t i = 0; i < 3 * tc; ++i)
        indices.push_back(ParseBinary_<GIndex>(bp));

    return true;
}
