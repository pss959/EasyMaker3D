#include "Commands/DeleteCommand.h"

std::string DeleteCommand::GetDescription() const {
    return "Deleted " + GetModelsDesc(GetModelNames());
}
