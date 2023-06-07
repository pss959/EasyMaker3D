#include "Widgets/AxisWidget.h"

#include "SG/Search.h"
#include "Util/Assert.h"
#include "Widgets/Slider2DWidget.h"
#include "Widgets/SphereWidget.h"

void AxisWidget::CreationDone() {
    CompositeWidget::CreationDone();

    if (! IsTemplate()) {
        // Access the rotation and translation widgets.
        rotator_    = AddTypedSubWidget<SphereWidget>("Rotator");
        translator_ = AddTypedSubWidget<Slider2DWidget>("Translator");

        // Access the parts needed for positioning and rotating.
        rotator_parts_ = SG::FindNodeUnderNode(*this,     "RotatorParts");
        cone_          = SG::FindNodeUnderNode(*rotator_, "Cone");
        base_          = SG::FindNodeUnderNode(*rotator_, "Base");

        // Set up callbacks.
        rotator_->GetRotationChanged().AddObserver(
            this, [&](Widget &, const Rotationf &){ AxisChanged_(true); });
        translator_->GetValueChanged().AddObserver(
            this, [&](Widget &, const Vector2f &){ AxisChanged_(false); });
    }
}

void AxisWidget::SetTranslationEnabled(bool enabled) {
    translator_->SetInteractionEnabled(enabled);
}

void AxisWidget::SetSize(float radius) {
    ASSERT(! rotator_->IsDragging() && ! translator_->IsDragging());
    const float kAxisScale = 1.6f;
    const float size = kAxisScale * radius;

    // Scale the translator shaft and position the arrow end parts.
    translator_->SetScale(Vector3f(1, .9f * size, 1));
    cone_->SetTranslation(Vector3f(0,       size, 0));
    base_->SetTranslation(Vector3f(0,      -size, 0));
}

void AxisWidget::SetTranslationRange(const Range2f &range) {
    ASSERT(! rotator_->IsDragging() && ! translator_->IsDragging());
    translator_->SetRange(Vector2f(range.GetMinPoint()),
                          Vector2f(range.GetMaxPoint()));
}

void AxisWidget::SetDirection(const Vector3f &direction) {
    const Rotationf rot = Rotationf::RotateInto(Vector3f::AxisY(), direction);

    rotator_->GetRotationChanged().EnableAll(false);
    rotator_->SetRotation(rot);
    rotator_->GetRotationChanged().EnableAll(true);
}

void AxisWidget::SetPosition(const Point3f &position) {
    // This may be called during translation, so make sure to take the current
    // offset into account. XXXX This is wrong?
    SetTranslation(position -
                   rotator_->GetRotation() * translator_->GetTranslation());
}

Vector3f AxisWidget::GetDirection() const {
    return rotator_->GetRotation() * Vector3f::AxisY();
}

Point3f AxisWidget::GetPosition() const {
    /// Add the Slider2DWidget's translation value (which is only non-zero
    /// during its interaction) to the current translation.
    return Point3f(GetTranslation() +
                   rotator_->GetRotation() * translator_->GetTranslation());
}

void AxisWidget::SubWidgetActivated(const std::string &name,
                                    bool is_activation) {
    if (name == "Translator" && ! is_activation) {
        // When the Slider2DWidget is deactivated, transfer the translation
        // from the Slider2DWidget to the AxisWidget and reset the
        // Slider2DWidget without notifying.
        SetTranslation(GetPosition());
        rotator_parts_->SetTranslation(Vector3f::Zero());
        translator_->GetValueChanged().EnableObserver(this, false);
        translator_->SetValue(Vector2f::Zero());
        translator_->GetValueChanged().EnableObserver(this, true);
    }

    CompositeWidget::SubWidgetActivated(name, is_activation);
}

void AxisWidget::AxisChanged_(bool is_rotation) {
    if (! is_rotation)
        rotator_parts_->SetTranslation(translator_->GetTranslation());

    axis_changed_.Notify(is_rotation);
}
