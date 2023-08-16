#include "Commands/CombineHullModelCommand.h"

bool CombineHullModelCommand::IsValid(std::string &details) {
    if (! CombineCommand::IsValid(details))
        return false;
    if (GetModelNames().size() < 1U) {
        details = "Need at least one model name";
        return false;
    }
    return true;
}

std::string CombineHullModelCommand::GetDescription() const {
    return "Create convex hull model " + GetResultName() + " from " +
        GetModelsDesc(GetModelNames());
}
