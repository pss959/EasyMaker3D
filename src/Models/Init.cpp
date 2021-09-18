#include "Models/Init.h"

#include "Models/BoxModel.h"
#include "Models/CylinderModel.h"
#include "Parser/Parser.h"

void RegisterModelTypes(Parser::Parser &parser) {
#define ADD_TYPE_(T) parser.RegisterObjectType(#T, []{ return new T; });
    ADD_TYPE_(BoxModel);
    ADD_TYPE_(CylinderModel);
#undef ADD_TYPE_
}
