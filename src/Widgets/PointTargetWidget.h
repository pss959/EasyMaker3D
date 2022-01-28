#pragma once

#include <memory>

#include "Targets/PointTarget.h"
#include "Widgets/TargetWidgetBase.h"

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

    virtual void StartDrag(const DragInfo &info) override;
    virtual void ContinueDrag(const DragInfo &info) override;
    virtual void EndDrag() override;

  protected:
    PointTargetWidget() {}

    virtual void AddFields() override;
    virtual bool IsValid(std::string &details) override;

    virtual void ShowExtraSnapFeedback(bool is_snapping);

  private:
    /// \name Parsed Fields
    ///@{
    Parser::ObjectField<PointTarget> target_{"target"};
    ///@}

    /// Updates the PointTargetWidget to match the given PointTarget.
    void UpdateFromTarget_(const PointTarget &target);

    friend class Parser::Registry;
};

typedef std::shared_ptr<PointTargetWidget> PointTargetWidgetPtr;
