#include "Commands/ChangeClipCommand.h"

Str ChangeClipCommand::GetDescription() const {
    return "Changed the clip plane of " + GetModelsDesc(GetModelNames()) +
        " to " + GetPlane().ToString();
}
