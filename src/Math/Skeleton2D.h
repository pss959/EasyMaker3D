#pragma once

#include <vector>

#include "Math/Skeleton.h"
#include "Math/Types.h"

class Polygon;

/// 2D straight skeleton. XXXX What and why.
///
/// \ingroup Math
class Skeleton2D : public Skeleton<Point2f> {
  public:
    /// Constructs the Skeleton2D for the given Polygon.
    void BuildForPolygon(const Polygon &poly);
};
