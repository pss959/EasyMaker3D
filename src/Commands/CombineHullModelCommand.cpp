#include "Commands/CombineHullModelCommand.h"

std::string CombineHullModelCommand::GetDescription() const {
    return BuildDescription("convex hull");
}
