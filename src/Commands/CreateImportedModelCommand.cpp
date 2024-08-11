//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Commands/CreateImportedModelCommand.h"

#include "Util/Enum.h"

void CreateImportedModelCommand::AddFields() {
    AddField(path_.Init("path"));

    CreateModelCommand::AddFields();
}

void CreateImportedModelCommand::CreationDone() {
    CreateModelCommand::CreationDone();
    if (path_.WasSet())
        path_ = FixPath(GetPath());
}

Str CreateImportedModelCommand::GetDescription() const {
    return BuildDescription("imported");
}
