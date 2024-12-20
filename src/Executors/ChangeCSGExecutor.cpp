//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Executors/ChangeCSGExecutor.h"

#include "Commands/ChangeCSGOperationCommand.h"
#include "Managers/NameManager.h"
#include "Managers/SelectionManager.h"
#include "Models/CSGModel.h"

void ChangeCSGExecutor::Execute(Command &command, Command::Op operation) {
    ExecData_ &data = GetExecData_(command);

    ChangeCSGOperationCommand &ccc =
        GetTypedCommand<ChangeCSGOperationCommand>(command);

    auto &context = GetContext();

    for (auto &pm: data.per_model) {
        CSGModel &csg = GetTypedModel<CSGModel>(pm.path_to_model);
        Str from_name, to_name;
        if (operation == Command::Op::kDo) {
            const Vector3f old_offset = csg.GetLocalCenterOffset();
            csg.SetOperation(ccc.GetNewOperation());
            from_name = pm.old_name;
            to_name   = pm.new_name;
            csg.SetTranslation(pm.old_translation -
                               old_offset + csg.GetLocalCenterOffset());
        }
        else {  // Undo.
            csg.SetOperation(pm.old_operation);
            from_name = pm.new_name;
            to_name   = pm.old_name;
            csg.SetTranslation(pm.old_translation);
        }
        csg.ChangeModelName(to_name, false);
        context.name_manager->Remove(from_name);
        context.name_manager->Add(to_name);
    }
    context.selection_manager->ReselectAll();
}

ChangeCSGExecutor::ExecData_ & ChangeCSGExecutor::GetExecData_(
    Command &command) {
    // Create the ExecData_ if not already done.
    if (! command.GetExecData()) {
        ChangeCSGOperationCommand &ccc =
            GetTypedCommand<ChangeCSGOperationCommand>(command);

        const auto &model_names = ccc.GetModelNames();
        ASSERT(! model_names.empty());

        // If the command was read in, there should be result names already
        // stored in the command. If not, create unique names.
        StrVec result_names = ccc.GetResultNames();
        if (result_names.empty()) {
            auto &nm = *GetContext().name_manager;
            result_names.reserve(model_names.size());
            const Str prefix = Util::EnumToWord(ccc.GetNewOperation());
            // Temporarily add each new name so that all subsequent names will
            // be unique.
            for (size_t i = 0; i < model_names.size(); ++i) {
                const Str name = CreateUniqueName(prefix);
                result_names.push_back(name);
                nm.Add(name);
            }
            ccc.SetResultNames(result_names);

            // Remove all of the new names; they will be readded when Execute()
            // is called.
            for (const auto &name: result_names)
                nm.Remove(name);
        }
        ASSERT(result_names.size() == model_names.size());

        ExecData_ *data = new ExecData_;
        data->per_model.resize(model_names.size());
        for (size_t i = 0; i < model_names.size(); ++i) {
            ExecData_::PerModel &pm = data->per_model[i];
            pm.path_to_model   = FindPathToModel(model_names[i]);
            CSGModel &csg = GetTypedModel<CSGModel>(pm.path_to_model);
            pm.old_operation   = csg.GetOperation();
            pm.old_name        = csg.GetName();
            pm.new_name        = result_names[i];
            pm.old_translation = csg.GetTranslation();
        }
        command.SetExecData(data);
    }
    return *static_cast<ExecData_ *>(command.GetExecData());
}
