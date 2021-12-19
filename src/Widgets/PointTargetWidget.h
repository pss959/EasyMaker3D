#pragma once

#include <memory>

#include "Widgets/TargetWidgetBase.h"

/// PointTargetWidget is a derived TargetWidgetBase for making a point on a
/// Model or the Stage a target for interactive snapping operations. It can be
/// dragged to any point on a Model, snapping to vertices, or any point on the
/// Stage, snapping to grid lines.
///
/// \ingroup Widgets
class PointTargetWidget : public TargetWidgetBase {
  public:
    virtual void StartDrag(const DragInfo &info) override;
    virtual void ContinueDrag(const DragInfo &info) override;
    virtual void EndDrag() override;

  protected:
    PointTargetWidget() {}

    virtual void ShowExtraSnapFeedback(bool is_snapping);

  private:
    friend class Parser::Registry;
};

typedef std::shared_ptr<PointTargetWidget> PointTargetWidgetPtr;
