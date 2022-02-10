#include "Executors/Executor.h"

#include "SG/Search.h"

void Executor::SetContext(std::shared_ptr<Context> &context) {
    // Make sure the Context has all fields set.
    ASSERT(context);
    ASSERT(context->animation_manager);
    ASSERT(context->color_manager);
    ASSERT(context->name_manager);
    ASSERT(context->selection_manager);
    ASSERT(context->target_manager);
    ASSERT(context->tooltip_func);
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

SelPath Executor::FindPathToModel(const std::string &name) {
    return SelPath(SG::FindNodePathUnderNode(GetContext().root_model,
                                             name, false));
}

void Executor::AddModelInteraction(Model &model) {
    const auto &context = GetContext();
    context.selection_manager->AttachClickToModel(model);
    model.SetTooltipFunc(context.tooltip_func);
}

void Executor::SetRandomModelColor(Model &model) {
    // Access the mesh so that it can be validated.
    model.GetMesh();
    model.SetColor(GetContext().color_manager->GetNextModelColor());
}
