#include "Commands/CreateRevSurfModelCommand.h"

#include "Util/Enum.h"

std::string CreateRevSurfModelCommand::GetDescription() const {
    return BuildDescription("surface of revolution");
}
