#include "Items/Inspector.h"

#include "Math/Linear.h"
#include "SG/Search.h"
#include "Util/Assert.h"

// Constants affecting Inspector behavior.
static const float kTargetSize_ = 8;
static const float kScaleMult_  = .1f;

void Inspector::Activate(const SG::NodePtr &node) {
    ASSERT(! IsEnabled());

    // Save the current (default) translation.
    saved_translation_ = GetTranslation();

    // Compute a reasonable scale.
    const Vector3f size = node->GetScaledBounds().GetSize();
    const float scale = kTargetSize_ / size[GetMaxElementIndex(size)];
    SetUniformScale(scale);

    // Compute translation to offset the node.
    SetTranslation(saved_translation_ - scale * node->GetTranslation());

    // Reset the rotation.
    SetRotation(Rotationf::Identity());

    AddChild(node);
    SetEnabled(true);
}

void Inspector::Deactivate() {
    ASSERT(IsEnabled());

    SetTranslation(saved_translation_);
    ClearChildren();
    SetEnabled(false);
}

void Inspector::ApplyScaleChange(float delta) {
    ASSERT(IsEnabled());
    const float scale =
        Clamp((1 + kScaleMult_ * delta) * GetScale()[0], .1f, 10);
    SetUniformScale(scale);

    // Compute translation to offset the node.
    ASSERT(GetChildCount() == 1U);
    SetTranslation(saved_translation_ - scale * GetChild(0)->GetTranslation());
}

void Inspector::ApplyRotation(const Rotationf &rot) {
    SetRotation(rot);
}

bool Inspector::IsGrippableEnabled() const {
    return IsEnabled();
}

void Inspector::UpdateGripInfo(GripInfo &info) {
    // XXXX
}

void Inspector::ActivateGrip(Hand hand, bool is_active) {
    // XXXX
}
