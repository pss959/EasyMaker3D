#include "Parser/Registry.h"
#include "Place/EdgeTarget.h"
#include "Place/PointTarget.h"

#define REGISTER_TYPE_(T) Parser::Registry::AddType<T>(#T)

void RegisterTargetTypes_();

void RegisterTargetTypes_() {
    REGISTER_TYPE_(EdgeTarget);
    REGISTER_TYPE_(PointTarget);
}

#undef REGISTER_TYPE_

