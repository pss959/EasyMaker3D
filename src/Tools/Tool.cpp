#include "Tools/Tool.h"

#include "Assert.h"

void Tool::SetContext(std::shared_ptr<Context> &context) {
    ASSERT(context);
    context_ = context;
}

bool Tool::CanBeUsedFor(const Selection &sel) const {
    return sel.HasAny() && CanAttach(sel);
}

void Tool::AttachToModel(const Selection &sel) {
    ASSERT(sel.HasAny());
    selection_ = sel;

    ASSERT(path_to_model_.empty());
    path_to_model_ = sel.GetPrimary();

    Attach(path_to_model_);
}

void Tool::DetachFromModel() {
    ASSERT(! path_to_model_.empty());
    Detach();
    path_to_model_.clear();
}

void Tool::ReattachToModel() {
    ASSERT(! path_to_model_.empty());
    Detach();
    Attach(path_to_model_);
}

ModelPtr Tool::GetModel() const {
    if (! path_to_model_.empty())
        return path_to_model_.GetModel();
    return ModelPtr();
}

Tool::Context & Tool::GetContext() const {
    ASSERT(context_);
    return *context_;
}

Matrix4f Tool::GetLocalToStageMatrix(const SelPath &path, bool is_inclusive) {
    Matrix4f m = Matrix4f::Identity();
    for (auto &node: path) {
        if (! is_inclusive && node == path.back())
            break;
        m *= node->GetModelMatrix();
    }
    return m;
}
