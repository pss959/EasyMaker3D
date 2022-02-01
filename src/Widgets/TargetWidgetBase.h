#pragma once

#include "CoordConv.h"
#include "SG/NodePath.h"
#include "Util/Notifier.h"
#include "Widgets/DraggableWidget.h"

/// TargetWidgetBase is an abstract base class for the PointTargetWidget and
/// EdgeTargetWidget classes that consolidates shared code.
class TargetWidgetBase : public DraggableWidget {
  public:
    /// Returns a Notifier that is invoked when the user drags some part of the
    /// widget to change it. It is passed the widget.
    Util::Notifier<Widget &> & GetChanged() { return changed_; }

    /// Sets a flag telling the widget whether to indicate that it is actively
    /// snapping or not. A CoordConv instance is provided to help convert the
    /// feedback to different coordinate systems.
    void ShowSnapFeedback(const CoordConv &cc, bool is_snapping);

    virtual void StartDrag(const DragInfo &info) override;
    virtual void ContinueDrag(const DragInfo &info) override;
    virtual void EndDrag() override;

  protected:
    /// Derived classes must implement this to tell the given receiver Widget
    /// to place the target according to the DragInfo.
    virtual void PlaceTarget(Widget &widget, const DragInfo &info) = 0;

    /// Indicates that interactive target placement is done. Derived classes
    /// can use this to turn off any interactive feedback they may have. The
    /// base class implements it to do nothing.
    virtual void EndTargetPlacement() {}

    /// Derived classes may override this to show or hide any extra feedback to
    /// indicate snapping. A CoordConv instance is provided to help convert the
    /// feedback to different coordinate systems. The base class defines it to
    /// do nothing.
    virtual void ShowExtraSnapFeedback(const CoordConv &cc, bool is_snapping) {}

    /// Notifies observers when something changes in the Widget.
    void NotifyChanged() { changed_.Notify(*this); }

    /// Returns the color to use for an active target.
    static Color GetActiveColor();

  private:
    /// Notifies when the widget is changed interactively.
    Util::Notifier<Widget &> changed_;

    /// Whether snap feedback is showing or not.
    bool snap_feedback_active_ = false;

    /// Widget returned by last call to GetReceiver().
    WidgetPtr prev_receiver_;

    /// Returns the Widget that should receive the target based on the given
    /// DragInfo. This may be null.
    WidgetPtr GetReceiver_(const DragInfo &info);
};
