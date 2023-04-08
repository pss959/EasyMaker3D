#include "Commands/ConvertTwistCommand.h"

std::string ConvertTwistCommand::GetDescription() const {
    return "Applied twist to " + GetModelsDesc(GetModelNames());
}
