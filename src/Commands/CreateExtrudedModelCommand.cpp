#include "Commands/CreateExtrudedModelCommand.h"

#include "Util/Enum.h"

Str CreateExtrudedModelCommand::GetDescription() const {
    return BuildDescription("extruded");
}
