//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Handlers/DragRectHandler.h"

#if ENABLE_DEBUG_FEATURES

#include <cmath>

#include "Base/Event.h"
#include "Math/Frustum.h"
#include "SG/Node.h"
#include "Util/Assert.h"

Handler::HandleCode DragRectHandler::HandleEvent(const Event &event) {
    ASSERT(rect_);

    bool handled = false;
    if (is_dragging_) {
        if (IsDragEnd_(event)) {
            is_dragging_ = false;
            rect_->SetEnabled(false);
            handled = true;

            // Print the result.
            PrintRect_(start_point2_, event.position2D);
        }
        else if (event.flags.Has(Event::Flag::kPosition2D)) {
            const Point3f cur_point3 = GetPoint3_(event);
            const Vector3f size = cur_point3 - start_point3_;
            rect_->SetScale(Vector3f(std::abs(size[0]), std::abs(size[1]), 1));
            rect_->TranslateTo(.5f * (start_point3_ + cur_point3));
            handled = true;
        }
    }
    else {
        if (IsDragStart_(event)) {
            is_dragging_ = true;
            start_point2_ = event.position2D;
            start_point3_ = GetPoint3_(event);
            rect_->SetEnabled(true);
            handled = true;
        }
    }
    return handled ? HandleCode::kHandledStop : HandleCode::kNotHandled;
}

bool DragRectHandler::IsDragStart_(const Event &event) {
    return
        event.flags.Has(Event::Flag::kButtonPress) &&
        event.device == Event::Device::kMouse &&
        event.button == Event::Button::kMouse1 &&
        event.modifiers.Has(Event::ModifierKey::kControl);
}

bool DragRectHandler::IsDragEnd_(const Event &event) {
    return
        event.flags.Has(Event::Flag::kButtonRelease) &&
        event.device == Event::Device::kMouse &&
        event.button == Event::Button::kMouse1;
}

Point3f DragRectHandler::GetPoint3_(const Event &event) const {
    // Use the frustum to compute a ray through the 2D point. The origin of the
    // ray is the point on the image plane.
    ASSERT(frustum_);
    return frustum_->BuildRay(event.position2D).origin;
}

void DragRectHandler::PrintRect_(const Point2f &p0, const Point2f &p1) {
    Range2f rect;
    rect.ExtendByPoint(p0);
    rect.ExtendByPoint(p1);
    const Point2f  &min  = rect.GetMinPoint();
    const Vector2f &size = rect.GetSize();
    std::cout << "====== RECT X Y W H: "
              << min[0] << " " << min[1] << " "
              << size[0] << " " << size[1] << "\n";
}

#endif
