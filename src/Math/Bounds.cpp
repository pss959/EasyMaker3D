//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Math/Bounds.h"

#include "Math/Linear.h"
#include "Util/Assert.h"

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

// LCOV_EXCL_START [debug only]
Str Bounds::ToString(bool use_min_max) const {
    if (use_min_max)
        return ("B["   + ToStr(GetMinPoint()) +
                " => " + ToStr(GetMaxPoint()) + "]");
    else
        return ("B[c="  + ToStr(GetCenter())  +
                " s="   + ToStr(GetSize())    + "]");
}
// LCOV_EXCL_STOP
