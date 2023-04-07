#include "Commands/ConvertTwistCommand.h"

std::string ConvertTwistCommand::GetDescription() const {
    return "Applied twisting to " + GetModelsDesc(GetModelNames());
}
