#include "Commands/ChangeMirrorCommand.h"

std::string ChangeMirrorCommand::GetDescription() const {
    return "Mirrored " + GetModelsDesc(GetModelNames()) + " across plane " +
        GetPlane().ToString();
}
