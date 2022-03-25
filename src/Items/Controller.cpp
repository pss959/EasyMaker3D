#include "Items/Controller.h"

#include "Managers/ColorManager.h"
#include "SG/Line.h"
#include "SG/Search.h"
#include "Util/Assert.h"
#include "Util/Enum.h"
#include "Util/General.h"

void Controller::SetHand(Hand hand) {
    hand_ = hand;
    if (hand == Hand::kLeft && guide_parent_)
        RotateGuides_();
}

void Controller::SetGripGuideType(GripGuideType type) {
    if (cur_guide_->GetGripGuideType() != type) {
        // Set and show the current guide, hiding the rest.
        for (auto &guide: guides_) {
            if (guide->GetGripGuideType() == type) {
                cur_guide_ = guide;
                guide->SetEnabled(true);
            }
            else {
                guide->SetEnabled(false);
            }
        }
    }
}

void Controller::ShowPointer(bool show) {
    if (pointer_node_)
        pointer_node_->SetFlagEnabled(Flag::kRender, show);
}

void Controller::ShowGrip(bool show) {
    if (grip_node_)
        grip_node_->SetEnabled(show);
}

void Controller::ShowPointerHover(bool show, const Point3f &pt) {
    if (pointer_hover_node_) {
        pointer_hover_node_->SetEnabled(show);
        if (show) {
            // Scale based on distance from controller to maintain a reasonable
            // size.
            const float distance = ion::math::Distance(Point3f::Zero(), pt);
            const float scale = .0002f * distance;
            pointer_hover_node_->SetUniformScale(scale);
            pointer_hover_node_->SetTranslation(pt);
        }
    }
}

void Controller::ShowGripHover(bool show, const Point3f &pt,
                               const Color &color) {
    if (grip_hover_node_) {
        grip_hover_node_->SetEnabled(show);
        if (show) {
            // Flip X for left hand.
            Point3f guide_pt = cur_guide_->GetHoverPoint();
            if (hand_ == Hand::kLeft)
                guide_pt[0] = -guide_pt[0];
            grip_hover_node_->SetBaseColor(color);
            grip_hover_line_->SetEndpoints(guide_pt, pt);
        }
    }
}

void Controller::ShowActive(bool is_active, bool is_grip) {
    if (is_active) {
        if (is_grip) {
            ShowPointer(false);
            is_grip_dragging = true;
        }
        else {
            pointer_node_->SetBaseColor(
                ColorManager::GetSpecialColor("LaserActiveColor"));
            ShowGrip(false);
        }
    }
    else {
        pointer_node_->SetBaseColor(
            ColorManager::GetSpecialColor("LaserInactiveColor"));
        ShowPointer(true);
        ShowGrip(true);
        is_grip_dragging = false;
    }
}

Vector3f Controller::GetGuideDirection() const {
    // The Rotation guide points forward when not grip dragging.
    if (! is_grip_dragging &&
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
    grip_hover_line_ =
        SG::FindTypedShapeInNode<SG::Line>(*grip_hover_node_, "Line");

    // Access the GripGuides parent node and rotate for the left controller.
    guide_parent_ = SG::FindNodeUnderNode(*grip_node_, "GripGuides");
    if (GetHand() == Hand::kLeft)
        RotateGuides_();

    // Add each of its children as a guide.
    for (auto &child: guide_parent_->GetChildren()) {
        GripGuidePtr guide = Util::CastToDerived<GripGuide>(child);
        ASSERT(guide);
        guides_.push_back(guide);
    }
    ASSERT(guides_.size() == Util::EnumCount<GripGuideType>());

    // Start with no guide (the kNone version).
    cur_guide_ = guides_[0];

    // Set the inactive colors.
    ShowActive(false, true);
    ShowActive(false, false);
}

void Controller::RotateGuides_() {
    ASSERT(guide_parent_);
    guide_parent_->SetRotation(
        Rotationf::FromAxisAndAngle(Vector3f::AxisZ(),
                                    Anglef::FromDegrees(180)));
}
