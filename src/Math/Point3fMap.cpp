#include "Math/Point3fMap.h"

#include <algorithm>

#include "Math/Linear.h"
#include "Util/General.h"

Point3fMap::Point3fMap(float precision) : precision_(precision) {
}

GIndex Point3fMap::Add(const Point3f &p, Point3f *pos) {
    GIndex index;
    const Point3f rp = Round_(p);

    auto it = map_.find(rp);
    if (it != map_.end()) {
        index = it->second;
    }
    else {
        index = map_.size();
        map_[rp] = index;
    }
    if (pos)
        *pos = rp;
    return index;
}

bool Point3fMap::Contains(const Point3f &p) const {
    return Util::MapContains(map_, Round_(p));
}

Point3f Point3fMap::Round_(const Point3f &p) const {
    if (precision_ <= 0)
        return p;
    Point3f rp;
    for (int i = 0; i < 3; ++i)
        rp[i] = RoundToPrecision(p[i], precision_);
    return rp;
}

std::vector<Point3f> Point3fMap::GetPoints() const {
    typedef std::pair<Point3f, GIndex> Pair_;
    std::vector<Pair_> pairs(map_.begin(), map_.end());
    std::sort(pairs.begin(), pairs.end(),
              [](const Pair_ &p0,
                 const Pair_ &p1){ return p0.second < p1.second; });
    return Util::ConvertVector<Point3f, Pair_>(
        pairs, [](const Pair_ &p){ return p.first; });
}
