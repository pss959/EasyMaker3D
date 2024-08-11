//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Tests/Executors/ExecutorTestBase.h"

#include "Managers/AnimationManager.h"
#include "Managers/ClipboardManager.h"
#include "Managers/CommandManager.h"
#include "Managers/NameManager.h"
#include "Managers/SelectionManager.h"
#include "Managers/SettingsManager.h"
#include "Managers/TargetManager.h"
#include "Models/RootModel.h"
#include "Widgets/EdgeTargetWidget.h"
#include "Widgets/PointTargetWidget.h"

Executor::Context & ExecutorTestBase::InitContext(Executor &exec,
                                                  bool init_targets) {
    Executor::ContextPtr context(new Executor::Context);

    const Str contents = init_targets ? R"(
  children: [<"nodes/ModelRoot.emd">,
             <"nodes/Widgets/PointTargetWidget.emd">,
             <"nodes/Widgets/EdgeTargetWidget.emd">])" :
  R"(children: [<"nodes/ModelRoot.emd">])";

    auto root = ReadRealNode<RootModel>(contents, "ModelRoot");

    context->animation_manager.reset(new AnimationManager);
    context->clipboard_manager.reset(new ClipboardManager);
    context->command_manager.reset(new CommandManager);
    context->name_manager.reset(new NameManager);
    context->selection_manager.reset(new SelectionManager);
    context->settings_manager.reset(new SettingsManager);
    context->target_manager.reset(new TargetManager(context->command_manager));

    context->root_model = root;
    context->selection_manager->SetRootModel(root);

    exec.SetTestContext(context);

    // Install the target widgets in the TargetManager if requested.
    if (init_targets) {
        auto ptw = SG::FindTypedNodeInScene<PointTargetWidget>(
            *GetScene(), "PointTargetWidget");
        auto etw = SG::FindTypedNodeInScene<EdgeTargetWidget>(
            *GetScene(), "EdgeTargetWidget");
        context->target_manager->InitTargets(ptw, etw);
    }

    return *context;
}
