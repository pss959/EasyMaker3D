#include "Commands/DeleteCommand.h"

Str DeleteCommand::GetDescription() const {
    return "Deleted " + GetModelsDesc(GetModelNames());
}
