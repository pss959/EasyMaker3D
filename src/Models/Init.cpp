#include "Models/Init.h"

#include "Models/BoxModel.h"
#include "Models/CylinderModel.h"
#include "Models/RevSurfModel.h"
#include "Models/SphereModel.h"
#include "Models/TorusModel.h"
#include "Parser/Parser.h"

void RegisterModelTypes(Parser::Parser &parser) {
#define ADD_TYPE_(T) parser.RegisterObjectType(#T, []{ return new T; });
    ADD_TYPE_(BoxModel);
    ADD_TYPE_(CylinderModel);
    ADD_TYPE_(RevSurfModel);
    ADD_TYPE_(SphereModel);
    ADD_TYPE_(TorusModel);
#undef ADD_TYPE_
}
