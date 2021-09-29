#include "Executors/Executor.h"

void Executor::SetContext(std::shared_ptr<Context> &context) {
    // Make sure the Context has all fields set.
    ASSERT(context);
    ASSERT(context->root_model);
    ASSERT(context->animation_manager);
    ASSERT(context->color_manager);
    ASSERT(context->name_manager);
    ASSERT(context->selection_manager);
    context_ = context;
}

void Executor::FixModelName(Model &model, const std::string &name) {
if (! name.empty() && model.GetName() != name) {
#if XXXX
    ModelManager modelMgr = GetContext().modelManager;
    if (modelMgr.FindModel(name) != null)
        throw new DuplicateModelNameException(name);
    // If the Model is not known to the ModelManager, it is safe to
    // just change its name locally.
    if (modelMgr.FindModel(model.name))
        modelMgr.ChangeModelName(model, name);
    else
        model.name = name;
#endif
 }
}

void Executor::AddClickToModel(Model &model) {
    GetContext().selection_manager->AttachClickToModel(model);
}

void Executor::SetRandomModelColor(Model &model) {
    model.SetColor(GetContext().color_manager->GetNextModelColor());
}
