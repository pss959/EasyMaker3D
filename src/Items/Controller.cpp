#include "Items/Controller.h"

#include "SG/Search.h"
#include "Util/Assert.h"
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

void Controller::ShowPointer(bool show) {
    if (pointer_node_)
        pointer_node_->SetEnabled(Flag::kRender, show);
}

void Controller::ShowGrip(bool show) {
    if (grip_node_)
        grip_node_->SetEnabled(Flag::kRender, show);
}

void Controller::ShowPointerHover(bool show, const Point3f &pt) {
    if (pointer_hover_node_) {
        pointer_hover_node_->SetEnabled(SG::Node::Flag::kRender, show);
        if (show) {
            // Scale based on distance from controller to maintain a reasonable
            // size.
            const float distance = ion::math::Distance(Point3f::Zero(), pt);
            const float scale = .0002f * distance;
            pointer_hover_node_->SetUniformScale(scale);
            pointer_hover_node_->SetTranslation(Vector3f(pt));
        }
    }
}

void Controller::ShowGripHover(bool show, const Point3f &pt,
                               const Color &color) {
    if (grip_hover_node_) {
        grip_hover_node_->SetEnabled(SG::Node::Flag::kRender, show);
        if (show) {
            grip_hover_node_->SetBaseColor(color);
            grip_hover_line_->SetEndpoints(cur_guide_->GetHoverPoint(), pt);
        }
    }
}

Vector3f Controller::GetGuideDirection() const {
    // The Rotation guide points forward when not active.
    const bool is_active = false;  // XXXX Need to know when grip dragging.
    if (! is_active &&
        cur_guide_->GetGripGuideType() == GripGuideType::kRotation)
        return -Vector3f::AxisZ();

    // All other cases: point away from the palm.
    else
        return hand_ == Hand::kLeft ? Vector3f::AxisX() : -Vector3f::AxisX();
}

void Controller::PostSetUpIon() {
    // Access the important nodes.
    pointer_node_       = SG::FindNodeUnderNode(*this, "LaserPointer");
    grip_node_          = SG::FindNodeUnderNode(*this, "Grip");
    pointer_hover_node_ = SG::FindNodeUnderNode(*this, "PointerHoverHighlight");
    grip_hover_node_    = SG::FindNodeUnderNode(*this, "GripHoverHighlight");

    // Access the Line shape for the grip hover so it can have its endpoints
    // adjusted for feedback.
    const auto &gh_shapes = grip_hover_node_->GetShapes();
    ASSERT(gh_shapes.size() == 1U);
    grip_hover_line_ = Util::CastToDerived<SG::Line>(gh_shapes[0]);
    ASSERT(grip_hover_line_);

    // Access the GripGuides parent node and add each of its children as a
    // guide.
    auto guide_parent = SG::FindNodeUnderNode(*grip_node_, "GripGuides");
    for (auto &child: guide_parent->GetChildren()) {
        GripGuidePtr guide = Util::CastToDerived<GripGuide>(child);
        ASSERT(guide);
        guides_.push_back(guide);
    }
    ASSERT(guides_.size() == Util::EnumCount<GripGuideType>());

    // Start with no guide (the kNone version).
    cur_guide_ = guides_[0];
}
