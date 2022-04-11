#include "Commands/ConvertClipCommand.h"

std::string ConvertClipCommand::GetDescription() const {
    return "Applied clip to " + GetModelsDesc(GetModelNames());
}
