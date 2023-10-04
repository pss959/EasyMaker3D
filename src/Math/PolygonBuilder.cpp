#include "Math/PolygonBuilder.h"

#include "Math/Linear.h"
#include "Math/Polygon.h"
#include "Util/Assert.h"
#include "Util/General.h"

// ----------------------------------------------------------------------------
// PolygonBuilder::Impl_ class.
// ----------------------------------------------------------------------------

class PolygonBuilder::Impl_ {
  public:
    void BeginOutline(size_t border_count);
    void BeginBorder(size_t point_count);
    void AddPoint(const Point2f &pos, bool is_on_curve);
    void AddPolygons(std::vector<Polygon> &polys, float complexity);

  private:
    /// One point of a Polygon border. Some points are on the border itself
    /// (is_on_curve == true), and some are control points for quadratic Bezier
    /// curves (is_on_curve == false).
    struct Point_ {
        Point2f pos;          ///< Position of the point.
        bool    is_on_curve;  ///< False if a Bezier control point.
        Point_(const Point2f &pos_in, bool is_on_curve_in) :
            pos(pos_in), is_on_curve(is_on_curve_in) {}
    };

    /// Outside border or hole border within an outline.
    struct Border_ {
        std::vector<Point_> points;  ///< Points forming the border.
        Range2f             bounds;  ///< Rectangular bounds of the points.
        Border_(size_t point_count) { points.reserve(point_count); }

        /// Returns true if the points in the Border_ are clockwise.
        bool IsClockwise() const {
            const auto to_p3d = [](const Point_ &p){
                return Point3f(p.pos[0], 0, p.pos[1]);
            };
            const std::vector<Point3f> pts =
                Util::ConvertVector<Point3f, Point_>(points, to_p3d);
            // The normal will point up if the points form a clockwise loop or
            // down for a counterclockwise loop.
            const Vector3f normal = ComputeNormal(pts);
            return normal[1] > 0;
        }
    };

    /// Outline consisting of one or more borders. The first border is assumed
    /// to be the outside, and the rest (if any) are assumed to be holes. This
    /// corresponds to a result Polygon.
    std::vector<Border_> borders_;

    /// Returns points forming a Border_, following TrueType curve rules. The
    /// complexity is used for subdividing curves.
    static std::vector<Point2f> ProcessBorder_(const Border_ &border,
                                               float complexity);

    /// Returns a simplified version of a Border_. This inserts on-curve points
    /// between each pair of consecutive off-curve points and makes sure the
    /// last point is on the curve.
    static Border_ SimplifyBorder_(const Border_ &border);

    /// Returns the index of the Border_ in the vector that contains the given
    /// hole border. Asserts if there isn't one.
    static size_t FindContainingBorderIndex_(
        const std::vector<Border_> &outer_borders, const Border_ &hole_border);

    /// Uses the De Casteljau algorithm to compute points along a quadratic
    /// Bezier curve, adding the resulting points (except p2) to the vector.
    static void AddQuadraticBezierPoints_(
        const Point2f &p0, const Point2f &p1, const Point2f &p2,
        float complexity, std::vector<Point2f> &pts);
};

void PolygonBuilder::Impl_::BeginOutline(size_t border_count) {
    borders_.reserve(border_count);
}

void PolygonBuilder::Impl_::BeginBorder(size_t point_count) {
    borders_.push_back(Border_(point_count));
}

void PolygonBuilder::Impl_::AddPoint(const Point2f &pos, bool is_on_curve) {
    ASSERT(! borders_.empty());
    borders_.back().points.push_back(Point_(pos, is_on_curve));
}

void PolygonBuilder::Impl_::AddPolygons(std::vector<Polygon> &polys,
                                        float complexity) {
    const size_t poly_start = polys.size();

    // Sort borders into outer borders and hole borders.
    std::vector<Border_> outer_borders;
    std::vector<Border_>  hole_borders;
    for (const Border_ &border: borders_) {
        if (border.IsClockwise())
            outer_borders.push_back(border);
        else
            hole_borders.push_back(border);
    }

    // Construct and add a Polygon for each outer border. Store the bounds in
    // the Border_.
    for (Border_ &border: outer_borders) {
        const Polygon poly(ProcessBorder_(SimplifyBorder_(border), complexity));
        polys.push_back(poly);
        border.bounds = poly.GetBoundingRect();
    }

    // Figure out which outer border contains each hole border and add the
    // border to the corresponding Polygon.
    for (const Border_ &hole: hole_borders) {
        const size_t index = FindContainingBorderIndex_(outer_borders, hole);
        polys[poly_start + index].AddHoleBorder(
            ProcessBorder_(SimplifyBorder_(hole), complexity));
    }
}

std::vector<Point2f> PolygonBuilder::Impl_::ProcessBorder_(const Border_ &border,
                                                           float complexity){
    //
    // Curve rules:
    //
    //   - A point is either On (is_on_curve) or Off (! is_on_curve).
    //
    //   - [On-On] is a line segment.
    //
    //   - [On-Off-On] is a conic Bezier. The Off point is the control
    //     point, and the first and third points are the start and end
    //     points.
    //
    //   - [Off-Off] is a shorthand for having the midpoint of the two Off
    //     points as a control point between them.
    //

    // I think the first point must be on the curve.
    ASSERT(! border.points.empty());
    ASSERT(border.points[0].is_on_curve);

    // Create the processed list of points.
    std::vector<Point2f> result_points;
    for (size_t i = 0; i < border.points.size(); ++i) {
        const Point_ pt = border.points[i];

        // On points are added directly. Off points must be surrounded by
        // On points to form a quadratic Bezier curve. Note that only
        // on-curve points are added if complexity is 0.
        if (pt.is_on_curve) {
            result_points.push_back(pt.pos);
        }
        else if (complexity > 0) {
            AddQuadraticBezierPoints_(border.points[i - 1].pos, pt.pos,
                                      border.points[i + 1].pos, complexity,
                                      result_points);
        }
    }

    // The order of outer vs. hole borders is the opposite for Polygon
    // (clockwise == hole). Reverse the points here.
    std::reverse(result_points.begin(), result_points.end());
    return result_points;
}

PolygonBuilder::Impl_::Border_ PolygonBuilder::Impl_::SimplifyBorder_(
    const Border_ &border) {
    ASSERT(! border.points.empty());

    Border_ simplified(border.points.size());
    for (size_t i = 0; i < border.points.size(); ++i) {
        const Point_ &pt = border.points[i];

        // If both this and the previous point are Off, add the midpoint
        // between them as an On point.
        if (! pt.is_on_curve) {
            const size_t prev = i == 0 ? border.points.size() - 1 : i - 1;
            const Point_ prev_pt = border.points[prev];
            if (! prev_pt.is_on_curve)
                simplified.points.push_back(Point_(.5f * (prev_pt.pos + pt.pos),
                                                   true));
        }

        // Add this point.
        simplified.points.push_back(pt);
    }
    // If the last point is Off, add the first point again.
    if (! simplified.points[simplified.points.size() - 1].is_on_curve)
        simplified.points.push_back(simplified.points[0]);

    return simplified;
}

size_t PolygonBuilder::Impl_::FindContainingBorderIndex_(
    const std::vector<Border_> &outer_borders, const Border_ &hole_border) {
    ASSERT(! hole_border.points.empty());
    const Point2f &hole_pt = hole_border.points[0].pos;
    for (size_t i = 0; i < outer_borders.size(); ++i) {
        if (outer_borders[i].bounds.ContainsPoint(hole_pt))
            return i;
    }
    // LCOV_EXCL_START [cannot happen]
    ASSERTM(false, "Can't find outer border containing hole");
    // LCOV_EXCL_STOP
    return 0;
}

void PolygonBuilder::Impl_::AddQuadraticBezierPoints_(
    const Point2f &p0, const Point2f &p1, const Point2f &p2,
    float complexity, std::vector<Point2f> &pts) {
    using ion::math::Length;

    // Compute the number of internal points along the curve based on the
    // complexity and lengths. Use a reasonable heuristic.
    const float kResFactor = 2;
    const float length = std::max(Length(p1 - p0), Length(p2 - p1));
    const size_t count = static_cast<int>(complexity * kResFactor * length);

    if (count == 0) {
        pts.push_back(p1);
    }
    else {
        const float delta_t = 1.f / (count + 1);
        for (size_t i = 0; i < count; ++i) {
            const float t = (i + 1) * delta_t;
            pts.push_back(Lerp(t, Lerp(t, p0, p1), Lerp(t, p1, p2)));
        }
    }
}

// ----------------------------------------------------------------------------
// PolygonBuilder functions.
// ----------------------------------------------------------------------------

PolygonBuilder::PolygonBuilder() : impl_(new Impl_) {
}

PolygonBuilder::~PolygonBuilder() {
}

void PolygonBuilder::BeginOutline(size_t border_count) {
    impl_->BeginOutline(border_count);
}

void PolygonBuilder::BeginBorder(size_t point_count) {
    impl_->BeginBorder(point_count);
}

void PolygonBuilder::AddPoint(const Point2f &pos, bool is_on_curve) {
    impl_->AddPoint(pos, is_on_curve);
}

void PolygonBuilder::AddPolygons(std::vector<Polygon> &polys,
                                 float complexity) {
    impl_->AddPolygons(polys, complexity);
}

FontSystem::OutlineFuncs PolygonBuilder::SetUpForText() {
    FontSystem::OutlineFuncs funcs;
    funcs.begin_outline_func = [&](size_t nc){ BeginOutline(nc); };
    funcs.begin_border_func  = [&](size_t np){ BeginBorder(np);  };
    funcs.add_point_func     = [&](float x, float y, bool is_on_curve){
        AddPoint(Point2f(x, y), is_on_curve); };
    return funcs;
}
