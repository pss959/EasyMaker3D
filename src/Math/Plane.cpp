//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Math/Plane.h"

#include <ion/math/vectorutils.h>

#include "Math/Linear.h"

Plane::Plane() : normal(Vector3f::AxisZ()), distance(0.f) {
}

Plane::Plane(float dist, const Vector3f &norm) :
    normal(ion::math::Normalized(norm)),
    distance(dist) {
}

Plane::Plane(const Point3f &point, const Vector3f &norm) :
    normal(ion::math::Normalized(norm)),
    distance(SignedDistance(point, normal)) {
}

Plane::Plane(const Point3f &p0, const Point3f &p1, const Point3f &p2) :
    normal(ion::math::Normalized(ion::math::Cross(p1 - p0, p2 - p0))),
    distance(SignedDistance(p0, normal)) {
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

Str Plane::ToString() const {
    return "PL [n=" + ToStr(normal) + " d=" + ToStr(distance) + "]";
}
