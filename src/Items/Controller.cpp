#include "Items/Controller.h"

#include "Assert.h"
#include "SG/Search.h"
#include "Util/Enum.h"
#include "Util/General.h"

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

void Controller::ShowGripHover(const Point3f &pt, const Color &color) {
    if (grip_hover_node_) {
        grip_hover_node_->SetBaseColor(color);
        grip_hover_line_->SetEndpoints(cur_guide_->GetHoverPoint(), pt);
        grip_hover_node_->SetEnabled(SG::Node::Flag::kRender, true);
    }
}

void Controller::HideGripHover() {
    if (grip_hover_node_) {
        grip_hover_node_->SetEnabled(SG::Node::Flag::kRender, false);
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

    // Start with no guide (the kNone version).
    cur_guide_ = guides_[0];

    // Save a pointer to the grip hover highlight Node and its Line shape.
    grip_hover_node_ = SG::FindNodeUnderNode(*this, "GripHoverHighlight");
    ASSERT(grip_hover_node_->GetShapes().size() == 1U);
    grip_hover_line_ =
        Util::CastToDerived<SG::Line>(grip_hover_node_->GetShapes()[0]);
    ASSERT(grip_hover_line_);
}

