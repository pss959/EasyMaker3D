#pragma once

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
    /// snapping or not.
    void ShowSnapFeedback(bool is_snapping);

  protected:
    /// Derived classes may override this to show or hide any extra feedback to
    /// indicate snapping. The base class defines it to do nothing.
    virtual void ShowExtraSnapFeedback(bool is_snapping) {}

    /// Derived classes can call this to find the widget that should receive
    /// the target based on the given DragInfo. This may be null.
    WidgetPtr GetReceiver(const DragInfo &info);

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
};
