#include "Items/Inspector.h"

#include "Math/Linear.h"
#include "Util/Assert.h"

void Inspector::Activate(const SG::NodePtr &node) {
    ASSERT(! IsEnabled());

    // Save the current (default) translation.
    saved_translation_ = GetTranslation();

    // Compute a reasonable scale.
    const float kTargetSize = 8;
    const Vector3f size = node->GetScaledBounds().GetSize();
    const float scale = kTargetSize / size[GetMaxElementIndex(size)];
    SetUniformScale(scale);

    // Compute translation to offset the node.
    SetTranslation(saved_translation_ - scale * node->GetTranslation());

    AddChild(node);
    SetEnabled(true);
}

void Inspector::Deactivate() {
    ASSERT(IsEnabled());

    SetTranslation(saved_translation_);
    ClearChildren();
    SetEnabled(false);
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
