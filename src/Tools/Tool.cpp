#include "Tools/Tool.h"

#include "Assert.h"

void Tool::SetContext(std::shared_ptr<Context> &context) {
    ASSERT(context);
    context_ = context;
}

bool Tool::CanBeUsedFor(const Selection &sel) const {
    return sel.HasAny() && CanAttach(sel);
}

void Tool::AttachToModel(const ModelPtr &model, const Selection &sel) {
    ASSERT(model);
    ASSERT(! model_);
    selection_ = sel;
    model_     = model;
    Attach(model);
}

void Tool::DetachFromModel() {
    ASSERT(model_);
    Detach();
    model_.reset();
}

void Tool::ReattachToModel() {
    ASSERT(model_);
    Detach();
    Attach(model_);
}

Tool::Context & Tool::GetContext() const {
    ASSERT(context_);
    return *context_;
}
