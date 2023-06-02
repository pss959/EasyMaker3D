#include "Parser/Registry.h"
#include "Widgets/AxisWidget.h"
#include "Widgets/DiscWidget.h"
#include "Widgets/EdgeTargetWidget.h"
#include "Widgets/GenericWidget.h"
#include "Widgets/IconSwitcherWidget.h"
#include "Widgets/IconWidget.h"
#include "Widgets/PlaneWidget.h"
#include "Widgets/PointTargetWidget.h"
#include "Widgets/PushButtonWidget.h"
#include "Widgets/RadialLayoutWidget.h"
#include "Widgets/ScaleWidget.h"
#include "Widgets/Slider1DWidget.h"
#include "Widgets/Slider2DWidget.h"
#include "Widgets/SphereWidget.h"
#include "Widgets/StageWidget.h"

#define REGISTER_TYPE_(T) Parser::Registry::AddType<T>(#T)

void RegisterWidgetTypes_();

void RegisterWidgetTypes_() {
    REGISTER_TYPE_(AxisWidget);
    REGISTER_TYPE_(DiscWidget);
    REGISTER_TYPE_(EdgeTargetWidget);
    REGISTER_TYPE_(GenericWidget);
    REGISTER_TYPE_(IconSwitcherWidget);
    REGISTER_TYPE_(IconWidget);
    REGISTER_TYPE_(PointTargetWidget);
    REGISTER_TYPE_(PlaneWidget);
    REGISTER_TYPE_(PushButtonWidget);
    REGISTER_TYPE_(RadialLayoutWidget);
    REGISTER_TYPE_(ScaleWidget);
    REGISTER_TYPE_(Slider1DWidget);
    REGISTER_TYPE_(Slider2DWidget);
    REGISTER_TYPE_(SphereWidget);
    REGISTER_TYPE_(StageWidget);
}

#undef REGISTER_TYPE_

