#include "Commands/ConvertBevelCommand.h"

std::string ConvertBevelCommand::GetDescription() const {
    return "Applied bevel to " + GetModelsDesc(GetModelNames());
}
