#include "Commands/ConvertBendCommand.h"

std::string ConvertBendCommand::GetDescription() const {
    return "Applied bend to " + GetModelsDesc(GetModelNames());
}
