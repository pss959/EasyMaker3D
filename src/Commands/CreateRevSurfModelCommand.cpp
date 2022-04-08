#include "Commands/CreateRevSurfModelCommand.h"

#include "Util/Enum.h"

std::string CreateRevSurfModelCommand::GetDescription() const {
    return "Create surface of revolution Model " + GetResultName();
}
