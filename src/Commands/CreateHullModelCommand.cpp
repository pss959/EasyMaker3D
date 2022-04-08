#include "Commands/CreateHullModelCommand.h"

bool CreateHullModelCommand::IsValid(std::string &details) {
    if (! CombineCommand::IsValid(details))
        return false;
    if (GetModelNames().size() < 1U) {
        details = "Need at least one model name";
        return false;
    }
    return true;
}

std::string CreateHullModelCommand::GetDescription() const {
    return "Create convex hull model " + GetResultName() + " from " +
        GetModelsDesc(GetModelNames());
}
