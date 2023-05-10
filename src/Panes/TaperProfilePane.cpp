#include "Panes/TaperProfilePane.h"

#include "Math/Profile.h"
#include "Util/Tuning.h"

TaperProfilePane::TaperProfilePane() {
}

TaperProfilePane::~TaperProfilePane() {
}

Range2f TaperProfilePane::GetMovablePointRange(Slider2DWidget &slider,
                                               const Profile &profile,
                                               size_t index) const {
    Point2f min;
    Point2f max;

    // First and last points are limited to constant Y values.
    if (index == 0) {
        min.Set(0, 1);
        max.Set(1, 1);
    }
    else if (index + 1 == profile.GetPointCount()) {
        min.Set(0, 0);
        max.Set(1, 0);
    }
    // All other points are constrained by their neighbors to be monotonically
    // decreasing in Y and to not reach 0 in X.
    else {
        min[0] = TK::kMinTaperProfileYDistance;
        max[0] = 1;

        const auto &pts = profile.GetPoints();
        const auto &prev = pts[index - 1];
        const auto &next = pts[index + 1];
        max[1] = prev[1] - TK::kMinTaperProfileYDistance;
        min[1] = next[1] + TK::kMinTaperProfileYDistance;
    }
    return Range2f(min, max);
}

bool TaperProfilePane::CanInsertPoint(const Profile &profile,
                                      size_t index) const {
    // There must be at least 2 times the minimum Y distance between the
    // neighbor points.
    ASSERT(index + 1 < profile.GetPointCount());
    const auto &pts = profile.GetPoints();
    const auto &prev = pts[index];
    const auto &next = pts[index + 1];
    return prev[1] - next[1] > 2 * TK::kMinTaperProfileYDistance;
}
