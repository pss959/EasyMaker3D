#include "Widgets/EdgeTargetWidget.h"

#include <ion/math/vectorutils.h>

#include "SG/Line.h"
#include "SG/Search.h"
#include "Util/Assert.h"

void EdgeTargetWidget::AddFields() {
    AddField(target_.Init("target"));

    TargetWidgetBase::AddFields();
}

bool EdgeTargetWidget::IsValid(Str &details) {
    if (! TargetWidgetBase::IsValid(details))
        return false;  // LCOV_EXCL_LINE [cannot happen]
    const auto &target = target_.GetValue();
    if (! target) {
        details = "Missing target";
        return false;
    }
    if (target->GetLength() == 0) {
        details = "Length is zero";
        return false;
    }
    return true;
}

void EdgeTargetWidget::CreationDone() {
    TargetWidgetBase::CreationDone();

    if (! IsTemplate()) {
        // Find parts.
        end0_ = SG::FindNodeUnderNode(*this, "End0");
        end1_ = SG::FindNodeUnderNode(*this, "End1");
        edge_ = SG::FindNodeUnderNode(*this, "Edge");

        UpdateFromTarget_(GetEdgeTarget());
    }
}

void EdgeTargetWidget::SetEdgeTarget(const EdgeTarget &target) {
    target_.GetValue()->CopyFrom(target);
    UpdateFromTarget_(target);
}

void EdgeTargetWidget::PlaceTarget(ITargetable &targetable,
                                   const DragInfo &info) {
    auto &target = target_.GetValue();
    Point3f position0, position1;
    targetable.PlaceEdgeTarget(info, target->GetLength(), position0, position1);

    // Update the EdgeTarget.
    target->SetPositions(position0, position1);

    // Update this widget to match the target.
    UpdateFromTarget_(*target);
}

void EdgeTargetWidget::UpdateFromTarget_(const EdgeTarget &target) {
    // Update the parts to span the new positions.
    const Point3f &pos0 = target.GetPosition0();
    const Point3f &pos1 = target.GetPosition1();

    // Compute the edge length in stage coordinates.
    const float length = ion::math::Distance(pos0, pos1);

    // Rotate the whole widget.
    const Rotationf rot = Rotationf::RotateInto(Vector3f::AxisY(), pos1 - pos0);
    SetRotation(rot);

    // The inverse is used to position the ends.
    const Rotationf inv_rot = -rot;

    // Scale and position the parts of the widget to span the edge.
    end0_->TranslateTo(inv_rot * pos0);
    end1_->TranslateTo(inv_rot * pos1);
    edge_->TranslateTo(inv_rot * (.5f * (pos0 + pos1)));
    edge_->SetScale(Vector3f(1, length, 1));
}
