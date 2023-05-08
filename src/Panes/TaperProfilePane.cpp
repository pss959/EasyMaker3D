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
    float ymin;
    float ymax;

    // First and last points are limited to constant Y values.
    if (index == 0) {
        ymin = ymax = 1;
    }
    else if (index + 1 == profile.GetPointCount()) {
        ymin = ymax = 0;
    }
    // All other points are constrained by their neighbors to be monotonically
    // decreasing in Y.
    else {
        const auto &pts = profile.GetPoints();
        const auto &prev = pts[index - 1];
        const auto &next = pts[index + 1];
        ymax = prev[1] - TK::kMinTaperProfileYDistance;
        ymin = next[1] + TK::kMinTaperProfileYDistance;
    }
    return Range2f(Point2f(0, ymin), Point2f(1, ymax));
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
