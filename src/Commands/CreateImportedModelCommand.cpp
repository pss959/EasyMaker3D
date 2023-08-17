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
