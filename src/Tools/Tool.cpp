#include "Tools/Tool.h"

#include "Assert.h"

void Tool::SetContext(std::shared_ptr<Context> &context) {
    ASSERT(context);
    context_ = context;
}

bool Tool::CanBeUsedFor(const Selection &sel) const {
    return sel.HasAny() && CanAttach(sel);
}

void Tool::AttachToSelection(const Selection &sel) {
    ASSERT(sel.HasAny());
    selection_ = sel;
    Attach();
}

void Tool::DetachFromSelection() {
    ASSERT(selection_.HasAny());
    Detach();
    selection_.Clear();
}

void Tool::ReattachToSelection() {
    ASSERT(selection_.HasAny());
    Detach();
    Attach();
}

ModelPtr Tool::GetPrimaryModel() const {
    ModelPtr model;
    if (selection_.HasAny())
        model = selection_.GetPrimary().GetModel();
    return model;
}

Tool::Context & Tool::GetContext() const {
    ASSERT(context_);
    return *context_;
}

Matrix4f Tool::GetLocalToStageMatrix(bool is_inclusive) const {
    ASSERT(selection_.HasAny());
    const SelPath &path = selection_.GetPrimary();
    Matrix4f m = Matrix4f::Identity();
    for (auto &node: path) {
        if (! is_inclusive && node == path.back())
            break;
        m *= node->GetModelMatrix();
    }
    return m;
}
