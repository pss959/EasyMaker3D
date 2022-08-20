#include "Handlers/DragRectHandler.h"

#if ENABLE_DEBUG_FEATURES

#include <cmath>

#include "App/SceneContext.h"
#include "Base/Event.h"
#include "SG/Node.h"
#include "Util/Assert.h"

bool DragRectHandler::HandleEvent(const Event &event) {
    ASSERT(scene_context_);

    bool handled = false;
    if (is_dragging_) {
        if (IsDragEnd_(event)) {
            is_dragging_ = false;
            scene_context_->debug_rect->SetEnabled(false);
            handled = true;

            // Print the result.
            PrintRect_(start_point2_, event.position2D);
        }
        else if (event.flags.Has(Event::Flag::kPosition2D)) {
            const Point3f cur_point3 = GetPoint3_(event);
            const Vector3f size = cur_point3 - start_point3_;
            scene_context_->debug_rect->SetScale(
                Vector3f(std::abs(size[0]), std::abs(size[1]), 1));
            scene_context_->debug_rect->SetTranslation(
                .5f * (start_point3_ + cur_point3));
            handled = true;
        }
    }
    else {
        if (IsDragStart_(event)) {
            is_dragging_ = true;
            start_point2_ = event.position2D;
            start_point3_ = GetPoint3_(event);
            scene_context_->debug_rect->SetEnabled(true);
            handled = true;
        }
    }
    return handled;
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
    return scene_context_->frustum.BuildRay(event.position2D).origin;
}

void DragRectHandler::PrintRect_(const Point2f &p0, const Point2f &p1) {
    Range2f rect;
    rect.ExtendByPoint(p0);
    rect.ExtendByPoint(p1);
    const Point2f  &min  = rect.GetMinPoint();
    const Vector2f &size = rect.GetSize();
    std::cout << "====== RECT X Y Z W: "
              << min[0] << " " << min[1] << " "
              << size[0] << " " << size[1] << "\n";
}

#endif
