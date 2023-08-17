#include "Commands/ChangeTwistCommand.h"

Str ChangeTwistCommand::GetDescription() const {
    return "Twisted " + GetModelsDesc(GetModelNames());
}
