#include "Executors/Executor.h"

#include "Managers/AnimationManager.h"
#include "Managers/ClipboardManager.h"
#include "Managers/CommandManager.h"
#include "Managers/NameManager.h"
#include "Managers/SelectionManager.h"
#include "Managers/SettingsManager.h"
#include "Managers/TargetManager.h"
#include "Models/RootModel.h"
#include "SG/ColorMap.h"
#include "SG/Search.h"

void Executor::SetContext(std::shared_ptr<Context> &context) {
    // Make sure the Context has all fields set.
    ASSERT(context);
    ASSERT(context->animation_manager);
    ASSERT(context->clipboard_manager);
    ASSERT(context->command_manager);
    ASSERT(context->name_manager);
    ASSERT(context->selection_manager);
    ASSERT(context->settings_manager);
    ASSERT(context->target_manager);
    ASSERT(context->tooltip_func);
    context_ = context;
}

std::string Executor::CreateUniqueName(const std::string &prefix) {
    return context_->name_manager->Create(prefix);
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
    model.SetColor(Model::GetNextColor());
}
