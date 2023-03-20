#include "Items/Inspector.h"

#include "Items/Controller.h"
#include "Math/Linear.h"
#include "SG/Node.h"
#include "SG/Search.h"
#include "Util/Assert.h"
#include "Util/Tuning.h"

void Inspector::Activate(const SG::NodePtr &node,
                         const ControllerPtr &controller) {
    ASSERT(! IsEnabled());

    // Apply a translation to center the Node on the origin.
    centerer_->SetTranslation(-node->GetTranslation());

    // Scale the Node to a reasonable scale.
    const float target_size = controller ?
        TK::kInspectorVRModelSize : TK::kInspectorNonVRModelSize;
    const Vector3f node_size = node->GetScaledBounds().GetSize();
    const float scale = target_size / node_size[GetMaxElementIndex(node_size)];
    scaler_->SetUniformScale(scale);

    // Add the Node.
    centerer_->AddChild(node);

    if (controller) {
        // If using a Controller, attach the scaled Node to it.
        const Vector3f offset(TK::kInspectorVRXOffset, 0, 0);
        controller->AttachObject(transformer_, offset);

        attached_controller_ = controller;
    }

    SetEnabled(true);
}

void Inspector::Deactivate() {
    ASSERT(IsEnabled());
    ASSERT(transformer_);
    if (attached_controller_) {
        attached_controller_->DetachObject(transformer_);
        attached_controller_.reset();
    }

    // Reset all possible transformation changes.
    centerer_->ResetTransform();
    scaler_->ResetTransform();
    transformer_->ResetTransform();

    centerer_->ClearChildren();

    SetEnabled(false);

    if (deactivation_func_)
        deactivation_func_();
}

void Inspector::SetPositionForView(const Frustum &frustum) {
    const Vector3f view_dir = frustum.orientation * -Vector3f::AxisZ();
    SetTranslation(frustum.position + TK::kInspectorViewDistance * view_dir);
}

void Inspector::ApplyScaleChange(float delta) {
    ASSERT(IsEnabled());
    if (! attached_controller_) {
        const float cur_scale = transformer_->GetScale()[0];
        const float scale =
            Clamp((1 + TK::kInspectorNonVRScaleMult * delta) * cur_scale,
                  TK::kInspectorNonVRMinScale, TK::kInspectorNonVRMaxScale);
        transformer_->SetUniformScale(scale);
    }
}

void Inspector::ApplyRotation(const Rotationf &rot) {
    ASSERT(transformer_);
    if (! attached_controller_)
        transformer_->SetRotation(rot);
}

void Inspector::ShowEdges(bool show) {
    auto &block = transformer_->GetUniformBlockForPass("Lighting");
    block.SetFloatUniformValue("uEdgeWidth", show ? 1 : 0);
}

void Inspector::PostSetUpIon() {
    // Access important Nodes.
    ASSERT(! transformer_);
    transformer_ = SG::FindNodeUnderNode(*this, "Transformer");
    scaler_      = SG::FindNodeUnderNode(*this, "Scaler");
    centerer_    = SG::FindNodeUnderNode(*this, "Centerer");
}
