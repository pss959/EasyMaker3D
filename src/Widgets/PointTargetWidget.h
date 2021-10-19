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

  protected:
    virtual void ShowExtraSnapFeedback(bool is_snapping) {}

  private:
};

typedef std::shared_ptr<PointTargetWidget> PointTargetWidgetPtr;
