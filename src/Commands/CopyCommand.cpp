#include "Commands/CopyCommand.h"

Str CopyCommand::GetDescription() const {
    return "Copied " + GetModelsDesc(GetModelNames());
}
