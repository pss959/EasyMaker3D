#include "Commands/ChangeBendCommand.h"

Str ChangeBendCommand::GetDescription() const {
    return "Bent " + GetModelsDesc(GetModelNames());
}
