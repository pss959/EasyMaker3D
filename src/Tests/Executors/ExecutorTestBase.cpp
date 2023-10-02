#include "Tests/Executors/ExecutorTestBase.h"

#include "Managers/ClipboardManager.h"
#include "Managers/CommandManager.h"
#include "Managers/NameManager.h"
#include "Managers/SelectionManager.h"
#include "Managers/SettingsManager.h"
#include "Managers/TargetManager.h"
#include "Models/RootModel.h"

Executor::Context & ExecutorTestBase::InitContext(Executor &exec,
                                                  const Str &extra_contents) {
    Executor::ContextPtr context(new Executor::Context);

    Str contents = R"(children: [<"nodes/ModelRoot.emd">])";
    if (! extra_contents.empty())
        contents += ", " + extra_contents;

    auto root = ReadRealNode<RootModel>(contents, "ModelRoot");

    context->clipboard_manager.reset(new ClipboardManager);
    context->command_manager.reset(new CommandManager);
    context->name_manager.reset(new NameManager);
    context->selection_manager.reset(new SelectionManager);
    context->settings_manager.reset(new SettingsManager);
    context->target_manager.reset(new TargetManager(context->command_manager));

    context->root_model = root;
    context->selection_manager->SetRootModel(root);

    exec.SetTestContext(context);

    return *context;
}
