#include "Items/GripGuide.h"

#include "Assert.h"
#include "SG/Search.h"
#include "Util/Enum.h"

void GripGuide::SetGuideType(GripGuideType type) {
    if (guide_type_ != type) {
        guide_type_ = type;

        // Show the current guide, hiding the rest.
        const int index = guide_type_ == GripGuideType::kNone ? -1 :
            Util::EnumInt(guide_type_);
        for (size_t i = 0; i < GetChildCount(); ++i)
            GetChild(i)->SetEnabled(Flag::kTraversal,
                                    static_cast<int>(i) == index);
    }
}

Point3f GripGuide::GetHoverPoint() const {
    return hover_points_[Util::EnumInt(guide_type_)];
}

void GripGuide::PostSetUpIon() {
    // Make sure each of the expected children is there and has a "HoverPoint"
    // node somewhere. Save the relative hover point locations.
    const size_t type_count = Util::EnumCount<GripGuideType>();
    ASSERT(GetChildCount() == type_count);
    hover_points_.push_back(Point3f::Zero());  // GripGuideType::kNone.
    for (auto &child: GetChildren()) {
        const SG::NodePath path = SG::FindNodePathUnderNode(child, "HoverPoint");
        hover_points_.push_back(path.FromLocal(Point3f::Zero()));
    }
}
