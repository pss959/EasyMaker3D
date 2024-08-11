//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include "Trackers/PointerTracker.h"
#include "Util/Memory.h"

DECL_SHARED_PTR(Frustum);
namespace SG { DECL_SHARED_PTR(Node); }

/// MouseTracker is a derived PointerTracker class that tracks the mouse.
///
/// \ingroup Trackers
class MouseTracker : public PointerTracker {
  public:
    explicit MouseTracker(Actuator actuator);

    /// Sets the Frustum used to build rays.
    void SetFrustum(const FrustumPtr &frustum) { frustum_ = frustum; }

    /// Sets the SG::Node containing the sphere used to help with debugging.
    void SetDebugSphere(const SG::NodePtr &ds);

    virtual Event::Device GetDevice() const override;
    virtual bool IsActivation(const Event &event, WidgetPtr &widget) override;
    virtual bool IsDeactivation(const Event &event, WidgetPtr &widget) override;
    virtual float GetClickTimeout() const override;

  protected:
    virtual bool GetRay(const Event &event, Ray &ray) override;
    virtual Anglef GetMinRayAngleChange() const override;
    virtual void ProcessCurrentHit(const SG::Hit &hit) override;

  private:
    FrustumPtr  frustum_;
    SG::NodePtr debug_sphere_;
};
