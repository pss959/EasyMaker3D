#pragma once

#include <memory>
#include <vector>

#include "Math/Types.h"

class Polygon;

/// The PolygonBuilder class is used to incrementally construct a set of
/// Polygons from a set of outlines, each of which consists of one or more
/// borders. This is used for text.
///
/// This code assumes that a clockwise border is an outer border and a
/// counterclockwise border represents a hole. Note that this is the exact
/// opposite of the Polygon class, but it corresponds to the ordering in the
/// Freetype2 library for text.
///
/// \ingroup Math
class PolygonBuilder {
  public:
    PolygonBuilder();
    ~PolygonBuilder();
    void BeginOutline(size_t border_count);
    void BeginBorder(size_t point_count);
    void AddPoint(const Point2f &pos, bool is_on_curve);
    void AddPolygons(std::vector<Polygon> &polys, float complexity);

  private:
    class Impl_;
    std::unique_ptr<Impl_> impl_;
};
