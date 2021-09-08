#include "Widgets/Init.h"

#include "Parser/Parser.h"
#include "Widgets/DiscWidget.h"
#include "Widgets/Slider1DWidget.h"

void RegisterWidgetTypes(Parser::Parser &parser) {
#define ADD_TYPE_(T) parser.RegisterObjectType(#T, []{ return new T; });
    ADD_TYPE_(DiscWidget);
    ADD_TYPE_(Slider1DWidget);
#undef ADD_TYPE_
}
