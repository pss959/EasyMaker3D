#pragma once

#include "SG/NodePath.h"
#include "Util/Notifier.h"
#include "Widgets/DraggableWidget.h"

/// TargetWidgetBase is an abstract base class for the PointTargetWidget and
/// EdgeTargetWidget classes that consolidates shared code.
class TargetWidgetBase : public DraggableWidget {
  public:
    /// Sets the path from the root of the scene to the Stage for converting
    /// between stage and world coordinates. This must be called before
    /// dragging a target widget.
    void SetStagePath(const SG::NodePath &path) { stage_path_ = path; }

    /// Returns a Notifier that is invoked when the user drags the widget to
    /// move it. It is passed the widget.
    Util::Notifier<Widget &> & GetMoved() { return moved_; }

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

    /// Returns the path from the root of the scene to the Stage for converting
    /// between stage and world coordinates.
    const SG::NodePath & GetStagePath() const { return stage_path_; }

  private:
    /// Notifies when the widget moves.
    Util::Notifier<Widget &> moved_;

    /// Path to the Stage for coordinate conversion.
    SG::NodePath stage_path_;

    /// Whether snap feedback is showing or not.
    bool snap_feedback_active_ = false;

    /// Widget returned by last call to GetReceiver().
    WidgetPtr prev_receiver_;
};
