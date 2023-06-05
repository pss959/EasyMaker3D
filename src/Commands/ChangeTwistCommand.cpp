#include "Commands/ChangeTwistCommand.h"

std::string ChangeTwistCommand::GetDescription() const {
    return "Twisted " + GetModelsDesc(GetModelNames());
}
