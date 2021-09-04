#include "Widgets/Init.h"

#include "Parser/Parser.h"
#include "Widgets/DiscWidget.h"

void RegisterWidgetTypes(Parser::Parser &parser) {
#define ADD_TYPE_(T) parser.RegisterObjectType(#T, []{ return new T; });
    ADD_TYPE_(DiscWidget);
#undef ADD_TYPE_
}
