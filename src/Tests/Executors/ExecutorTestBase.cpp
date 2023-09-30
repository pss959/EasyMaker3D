#include "Tests/Executors/ExecutorTestBase.h"

#include "Managers/CommandManager.h"
#include "Managers/NameManager.h"
#include "Managers/SelectionManager.h"
#include "Managers/TargetManager.h"
#include "Models/RootModel.h"

Executor::Context & ExecutorTestBase::InitContext(Executor &exec) {
    Executor::ContextPtr context(new Executor::Context);

    auto root = ReadRealNode<RootModel>(
        R"(children: [<"nodes/ModelRoot.emd">])", "ModelRoot");

    context->root_model = root;
    context->command_manager.reset(new CommandManager);
    context->name_manager.reset(new NameManager);
    context->selection_manager.reset(new SelectionManager);
    context->selection_manager->SetRootModel(root);
    context->target_manager.reset(new TargetManager(context->command_manager));

    exec.SetTestContext(context);

    return *context;
}
