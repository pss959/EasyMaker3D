#include "Models/Init.h"

#include "Parser/Parser.h"

void RegisterModelTypes(Parser::Parser &parser) {
#define ADD_TYPE_(T) parser.RegisterObjectType(#T, []{ return new T; });
#undef ADD_TYPE_
}
