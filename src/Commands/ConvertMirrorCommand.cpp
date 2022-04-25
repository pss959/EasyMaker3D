#include "Commands/ConvertMirrorCommand.h"

std::string ConvertMirrorCommand::GetDescription() const {
    return "Applied mirroring to " + GetModelsDesc(GetModelNames());
}
