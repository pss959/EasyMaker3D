//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include "Math/Dimensionality.h"
#include "Place/PointTarget.h"
#include "Util/Memory.h"
#include "Widgets/TargetWidgetBase.h"

namespace SG {
DECL_SHARED_PTR(Tube);
DECL_SHARED_PTR(Node);
}

DECL_SHARED_PTR(PointTargetWidget);
DECL_SHARED_PTR(RadialLayoutWidget);

/// PointTargetWidget is a derived TargetWidgetBase for making a point on a
/// Model or the Stage a target for interactive snapping operations. It can be
/// dragged to any point on a Model, snapping to vertices, or any point on the
/// Stage, snapping to grid lines.
///
/// \ingroup Widgets
class PointTargetWidget : public TargetWidgetBase {
  public:
    /// Returns the current point target.
    const PointTarget & GetPointTarget() const { return *target_.GetValue(); }

    /// Sets the point target to match the given one.
    void SetPointTarget(const PointTarget &target);

    /// Sets the point in stage coordinates for line feedback when
    /// snapping. The line goes from the target center to the point.
    void SetSnapFeedbackPoint(const Point3f &pt) { line_end_pt_ = pt; }

  protected:
    PointTargetWidget() {}

    virtual void AddFields() override;
    virtual bool IsValid(Str &details) override;
    virtual void CreationDone() override;

    virtual void PlaceTarget(ITargetable &targetable,
                             const DragInfo &info) override;
    virtual void StartTargetPlacement() override;
    virtual void EndTargetPlacement() override;
    virtual void ShowExtraSnapFeedback(bool is_snapping) override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::ObjectField<PointTarget> target_;
    ///@}

    // Parts of the widget.
    RadialLayoutWidgetPtr layout_widget_;
    SG::NodePtr snap_indicator_;  ///< Sphere showing target snapping.
    SG::NodePtr feedback_;        ///< Node with line for showing feedback.
    SG::TubePtr feedback_line_;   ///< Line for showing feedback.

    /// Other endpoint of the feedback line in stage coordinates.
    Point3f line_end_pt_{0, 0, 0};

    /// Updates the PointTargetWidget to match the given PointTarget.
    void UpdateFromTarget_(const PointTarget &target);

    /// Updates the snap indicator based on the given snapped dimensions.
    void SetSnapIndicator_(const Dimensionality &snapped_dims);

    void UpdateLayoutWidget_();
    void LayoutWidgetActivated_(bool is_activation);
    void LayoutWidgetChanged_();

    friend class Parser::Registry;
};
