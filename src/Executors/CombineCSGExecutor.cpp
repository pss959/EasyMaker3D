//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Executors/CombineCSGExecutor.h"

#include "Commands/CombineCSGModelCommand.h"
#include "Models/CSGModel.h"

CombinedModelPtr CombineCSGExecutor::CreateCombinedModel(
    Command &command, const Str &name) {
    CombineCSGModelCommand &cc =
        GetTypedCommand<CombineCSGModelCommand>(command);
    const CSGOperation op = cc.GetOperation();

    CSGModelPtr csg = Model::CreateModel<CSGModel>(
        name.empty() ? CreateUniqueName(Util::EnumToWord(op)) : name);
    csg->SetOperation(op);

    return csg;
}
