#include "Commands/ConvertTaperCommand.h"

std::string ConvertTaperCommand::GetDescription() const {
    return "Applied taper to " + GetModelsDesc(GetModelNames());
}
