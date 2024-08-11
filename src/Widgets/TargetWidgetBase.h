//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include "Math/Color.h"
#include "Math/Types.h"
#include "SG/NodePath.h"
#include "Util/Memory.h"
#include "Util/Notifier.h"
#include "Widgets/DraggableWidget.h"
#include "Widgets/ITargetable.h"

DECL_SHARED_PTR(TargetWidgetBase);

/// TargetWidgetBase is an abstract base class for the PointTargetWidget and
/// EdgeTargetWidget classes that consolidates shared code.
///
/// \ingroup Widgets
class TargetWidgetBase : public DraggableWidget {
  public:
    /// Returns a Notifier that is invoked when the user drags some part of the
    /// widget to change it. It is passed the widget.
    Util::Notifier<Widget &> & GetChanged() { return changed_; }

    /// Sets a matrix converting from stage to world coordinates.
    void SetStageToWorldMatrix(const Matrix4f &swm) { stage_to_world_ = swm; }

    /// Sets a flag telling the widget whether to indicate that it is actively
    /// snapping or not.
    void ShowSnapFeedback(bool is_snapping);

    virtual void StartDrag(const DragInfo &info) override;
    virtual void ContinueDrag(const DragInfo &info) override;
    virtual void EndDrag() override;

  protected:
    virtual void CreationDone() override;

    /// Returns a matrix converting from stage to world coordinates.
    const Matrix4f & GetStageToWorldMatrix() const { return stage_to_world_; }

    /// Derived classes must implement this to tell the given ITargetable to
    /// place the target according to the DragInfo.
    virtual void PlaceTarget(ITargetable &targetable, const DragInfo &info) = 0;

    /// Indicates that interactive target placement is starting. The base class
    /// implements it to do nothing.
    virtual void StartTargetPlacement() {}

    /// Indicates that interactive target placement is done. The base class
    /// implements it to do nothing.
    virtual void EndTargetPlacement() {}

    /// Derived classes may override this to show or hide any extra feedback to
    /// indicate snapping.The base class defines it to do nothing.
    virtual void ShowExtraSnapFeedback(bool is_snapping) {}

    /// Notifies observers when something changes in the Widget.
    void NotifyChanged() { changed_.Notify(*this); }

    /// Returns the color to use for an active target.
    static Color GetActiveColor();

  private:
    /// Notifies when the widget is changed interactively.
    Util::Notifier<Widget &> changed_;

    Matrix4f stage_to_world_ = Matrix4f::Identity();

    /// Whether snap feedback is showing or not.
    bool snap_feedback_active_ = false;
};
