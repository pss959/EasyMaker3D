#include "Tests/Executors/ExecutorTestBase.h"

#include "Managers/NameManager.h"
#include "Managers/SelectionManager.h"
#include "Models/RootModel.h"

Executor::Context & ExecutorTestBase::InitContext(Executor &exec) {
    Executor::ContextPtr context(new Executor::Context);

    auto root = ReadRealNode<RootModel>(
        R"(children: [<"nodes/ModelRoot.emd">])", "ModelRoot");

    context->root_model = root;
    context->name_manager.reset(new NameManager);
    context->selection_manager.reset(new SelectionManager);
    context->selection_manager->SetRootModel(root);

    exec.SetTestContext(context);

    return *context;
}
