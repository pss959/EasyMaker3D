//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include "Place/EdgeTarget.h"
#include "Util/Memory.h"
#include "Widgets/TargetWidgetBase.h"

DECL_SHARED_PTR(EdgeTargetWidget);

/// EdgeTargetWidget is a derived TargetWidgetBase for making a edge of a
/// Model a target for interactive snapping operations. It snaps to Model edges
/// and cannot be placed anywhere else on a Model. When dragged on the Stage,
/// it points upwards.
///
/// Clicking on the widget reverses its direction. The direction is used when
/// laying objects out linearly.
///
/// \ingroup Widgets
class EdgeTargetWidget : public TargetWidgetBase {
  public:
    /// Returns the current edge target.
    const EdgeTarget & GetEdgeTarget() const { return *target_.GetValue(); }

    /// Sets the edge target to match the given one.
    void SetEdgeTarget(const EdgeTarget &target);

  protected:
    EdgeTargetWidget() {}

    virtual void AddFields() override;
    virtual bool IsValid(Str &details) override;
    virtual void CreationDone() override;

    virtual void PlaceTarget(ITargetable &targetable,
                             const DragInfo &info) override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::ObjectField<EdgeTarget> target_;
    ///@}

    // Parts of the widget.
    SG::NodePtr end0_;  ///< Part showing starting position.
    SG::NodePtr end1_;  ///< Part showing ending position.
    SG::NodePtr edge_;  ///< Part connecting two ends.

    /// Updates the EdgeTargetWidget to match the given EdgeTarget.
    void UpdateFromTarget_(const EdgeTarget &target);

    friend class Parser::Registry;
};
