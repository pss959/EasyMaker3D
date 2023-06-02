#include "Widgets/AxisWidget.h"

#include "SG/Search.h"
#include "Widgets/Slider2DWidget.h"
#include "Widgets/SphereWidget.h"

void AxisWidget::CreationDone() {
    CompositeWidget::CreationDone();

    if (! IsTemplate()) {
        // Access the rotation and translation widgets.
        rotator_ = Util::CastToDerived<SphereWidget>(AddSubWidget("Rotator"));
        translator_ =
            Util::CastToDerived<Slider2DWidget>(AddSubWidget("Translator"));

        // Access the parts needed for rotating.
        cone_  = SG::FindNodeUnderNode(*rotator_, "Cone");
        base_  = SG::FindNodeUnderNode(*rotator_, "Base");

        // Set up callbacks.
        rotator_->GetRotationChanged().AddObserver(
            this, [&](Widget &, const Rotationf &){ RotationChanged_(); });
        translator_->GetValueChanged().AddObserver(
            this, [&](Widget &, const Vector2f &){ TranslationChanged_(); });
    }
}

void AxisWidget::SetTranslationEnabled(bool enabled) {
    translator_->SetInteractionEnabled(enabled);
}

void AxisWidget::SetDirection(const Vector3f &direction) {
    rotator_->GetRotationChanged().EnableAll(false);
    rotator_->SetRotation(Rotationf::RotateInto(Vector3f::AxisY(),
                                                direction));
    rotator_->GetRotationChanged().EnableAll(true);
}

void AxisWidget::SetPosition(const Point3f &position) {
    translator_->GetValueChanged().EnableAll(false);
    translator_->SetValue(Vector2f(position[0], position[2]));
    translator_->GetValueChanged().EnableAll(true);
}

Vector3f AxisWidget::GetDirection() const {
    return rotator_->GetRotation() * Vector3f::AxisY();
}

Point3f AxisWidget::GetPosition() const {
    const auto &v = translator_->GetValue();
    return rotator_->GetRotation() * Point3f(v[0], 0, v[1]);
}

void AxisWidget::SetSize(float radius) {
    const float kAxisScale = 1.6f;
    const float size = kAxisScale * radius;

    // Scale the translator shaft and position the arrow end parts.
    translator_->SetScale(Vector3f(1, size, 1));
    cone_->SetTranslation(Vector3f(0,  size, 0));
    base_->SetTranslation(Vector3f(0, -size, 0));
}

void AxisWidget::SetTranslationRange(const Range2f &range) {
    translator_->SetRange(Vector2f(range.GetMinPoint()),
                          Vector2f(range.GetMaxPoint()));
}

void AxisWidget::HighlightRotator(const Color &color) {
    HighlightSubWidget("Rotator", color);
}

void AxisWidget::UnhighlightRotator() {
    UnhighlightSubWidget("Rotator");
}

void AxisWidget::HighlightTranslator(const Color &color) {
    HighlightSubWidget("Translator", color);
}

void AxisWidget::UnhighlightTranslator() {
    UnhighlightSubWidget("Translator");
}

void AxisWidget::RotationChanged_() {
    axis_changed_.Notify(true);
}

void AxisWidget::TranslationChanged_() {
    axis_changed_.Notify(false);
}
