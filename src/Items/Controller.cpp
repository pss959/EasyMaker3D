#include "Items/Controller.h"

#include "Assert.h"
#include "SG/Search.h"
#include "Util/Enum.h"

void Controller::SetGripGuideType(GripGuideType type) {
    if (cur_guide_->GetGripGuideType() != type) {
        // Set and show the current guide, hiding the rest.
        for (auto &guide: guides_) {
            if (guide->GetGripGuideType() == type) {
                cur_guide_ = guide;
                guide->SetEnabled(Flag::kTraversal, true);
            }
            else {
                guide->SetEnabled(Flag::kTraversal, false);
            }
        }
    }
}

void Controller::PostSetUpIon() {
    // Access the GripGuides parent.
    auto guide_parent = SG::FindNodeUnderNode(*this, "GripGuides");

    // Add each child as a guide.
    for (auto &child: guide_parent->GetChildren()) {
        GripGuidePtr guide = Util::CastToDerived<GripGuide>(child);
        ASSERT(guide);
        guides_.push_back(guide);
    }
    ASSERT(guides_.size() == Util::EnumCount<GripGuideType>());
}

