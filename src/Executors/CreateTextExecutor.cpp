//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Executors/CreateTextExecutor.h"

#include "Commands/CreateTextModelCommand.h"
#include "Models/TextModel.h"

ModelPtr CreateTextExecutor::CreateModel(Command &command) {
    CreateTextModelCommand &ctc =
        GetTypedCommand<CreateTextModelCommand>(command);

    Str name = ctc.GetResultName();
    if (name.empty()) {
        name = CreateUniqueName("Text");
        ctc.SetResultName(name);
    }

    TextModelPtr tm = Model::CreateModel<TextModel>(name);
    tm->SetTextString(ctc.GetText());

    return tm;
}
