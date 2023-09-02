#pragma once

#include <memory>
#include <vector>

#include "Math/Types.h"
#include "Util/FontSystem.h"

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

    /// Begins a new outline containing the given number of borders.
    void BeginOutline(size_t border_count);

    /// Begins a new outline border containing the given number of points.
    void BeginBorder(size_t point_count);

    /// Adds a point to the current border.
    void AddPoint(const Point2f &pos, bool is_on_curve);

    /// Adds the resulting polygon(s) to the given vector, using \p complexity
    /// to compute any curves.
    void AddPolygons(std::vector<Polygon> &polys, float complexity);

    /// Sets up to use a PolygonBuilder to store text character outlines. This
    /// returns a FontSystem::OutlineFuncs instance that adds to the
    /// PolygonBuilder.
    FontSystem::OutlineFuncs SetUpForText();

  private:
    class Impl_;
    std::unique_ptr<Impl_> impl_;
};
