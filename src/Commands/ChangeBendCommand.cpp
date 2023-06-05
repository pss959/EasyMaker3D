#include "Commands/ChangeBendCommand.h"

std::string ChangeBendCommand::GetDescription() const {
    return "Bent " + GetModelsDesc(GetModelNames());
}
