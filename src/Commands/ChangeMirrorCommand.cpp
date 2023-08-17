#include "Commands/ChangeMirrorCommand.h"

Str ChangeMirrorCommand::GetDescription() const {
    return "Mirrored " + GetModelsDesc(GetModelNames()) + " across plane " +
        GetPlane().ToString();
}
