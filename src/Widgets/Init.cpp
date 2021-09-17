#include "Widgets/Init.h"

#include "Parser/Parser.h"
#include "Widgets/DiscWidget.h"
#include "Widgets/PushButtonWidget.h"
#include "Widgets/Slider1DWidget.h"
#include "Widgets/ToggleButtonWidget.h"

void RegisterWidgetTypes(Parser::Parser &parser) {
#define ADD_TYPE_(T) parser.RegisterObjectType(#T, []{ return new T; });
    ADD_TYPE_(DiscWidget);
    ADD_TYPE_(PushButtonWidget);
    ADD_TYPE_(Slider1DWidget);
    ADD_TYPE_(ToggleButtonWidget);
#undef ADD_TYPE_
}
