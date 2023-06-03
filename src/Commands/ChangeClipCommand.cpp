#include "Commands/ChangeClipCommand.h"

std::string ChangeClipCommand::GetDescription() const {
    return "Clipped " + GetModelsDesc(GetModelNames()) + " by plane " +
        GetPlane().ToString();
}
