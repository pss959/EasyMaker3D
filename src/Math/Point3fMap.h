//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include <map>
#include <vector>

#include "Math/Types.h"

/// A Point3fMap can be used when constructing a TriMesh or PolyMesh to ensure
/// that vertices are not duplicated. It creates a unique index (starting at 0
/// and incrementing by 1) for all Point3f instances in the map.
///
/// The precision parameter supplied to the constructor is used to optionally
/// round all coordinate values so that very close points will be considered
/// identical. Passing 0 bypasses this option.
///
/// \ingroup Math
class Point3fMap {
  public:
    /// The constructor is passed a precision value that is used to round each
    /// coordinate of every point if non-zero. This can be used to merge points
    /// that are very close to each other.
    Point3fMap(float precision);

    /// Adds a point if it is not already present. Returns the new index if it
    /// was added or the old index if it was already in the map. If the \c pos
    /// pointer is not null, the position of the new point adjusted by
    /// precision is returned.
    GIndex Add(const Point3f &p, Point3f *pos = nullptr);

    /// Returns true if the given point is close to one already in the map
    /// within the precision.
    bool Contains(const Point3f &p) const;

    /// Returns the number of vertices in the map.
    size_t GetCount() const { return map_.size(); }

    /// Returns all vertices added to the map as an ordered vector.
    std::vector<Point3f> GetPoints() const;

  private:
    /// Struct used to compare Point3f instances for the map.
    struct ComparePoints_ {
        bool operator()(const Point3f &p0, const Point3f &p1) const {
            for (int i = 0; i < 3; ++i) {
                if (p0[i] < p1[i])
                    return true;
                else if (p0[i] > p1[i])
                    return false;
            }
            return false;
        }
    };
    /// Precision value passed to the constructor.
    float precision_;

    /// Maps points to their indices. Use an std::map here instead of an
    /// std::unordered_map because there is no great hash function for a 3D
    /// point; it's much easier to define a comparison function that works.
    std::map<Point3f, GIndex, ComparePoints_> map_;

    /// Rounds by the precision if it is not 0.
    Point3f Round_(const Point3f &p) const;
};
