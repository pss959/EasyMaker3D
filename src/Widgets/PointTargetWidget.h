#pragma once

#include "Memory.h"
#include "Targets/PointTarget.h"
#include "Widgets/TargetWidgetBase.h"

namespace SG {
DECL_SHARED_PTR(Line);
DECL_SHARED_PTR(Node);
}

DECL_SHARED_PTR(PointTargetWidget);

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
    virtual bool IsValid(std::string &details) override;
    virtual void CreationDone() override;

    virtual void PlaceTarget(Widget &widget, const DragInfo &info) override;
    virtual void EndTargetPlacement() override;
    virtual void ShowExtraSnapFeedback(const CoordConv &stage_cc,
                                       bool is_snapping) override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::ObjectField<PointTarget> target_{"target"};
    ///@}

    // Parts of the widget.
    SG::NodePtr snap_indicator_;  ///< Sphere showing target snapping.
    SG::NodePtr feedback_;        ///< Node with line for showing feedback.
    SG::LinePtr feedback_line_;   ///< Line for showing feedback.

    /// Other endpoint of the feedback line in stage coordinates.
    Point3f line_end_pt_{0, 0, 0};

    /// Updates the PointTargetWidget to match the given PointTarget.
    void UpdateFromTarget_(const PointTarget &target);

    /// Updates the snap indicator based on the given snapped dimensions.
    void SetSnapIndicator_(const Dimensionality &snapped_dims);

    friend class Parser::Registry;
};
