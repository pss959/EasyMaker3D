#include "Commands/ChangeClipCommand.h"

std::string ChangeClipCommand::GetDescription() const {
    return "Changed the clip plane of " + GetModelsDesc(GetModelNames()) +
        " to " + GetPlane().ToString();
}
