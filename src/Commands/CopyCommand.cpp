#include "Commands/CopyCommand.h"

std::string CopyCommand::GetDescription() const {
    return "Copied " + GetModelsDesc(GetModelNames());
}
