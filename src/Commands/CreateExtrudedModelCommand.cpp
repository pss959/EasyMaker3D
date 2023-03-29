#include "Commands/CreateExtrudedModelCommand.h"

#include "Util/Enum.h"

std::string CreateExtrudedModelCommand::GetDescription() const {
    return "Create extruded Model " + GetResultName();
}
