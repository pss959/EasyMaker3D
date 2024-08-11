//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Items/AppInfo.h"
#include "Items/Border.h"
#include "Items/BuildVolume.h"
#include "Items/Controller.h"
#include "Items/Frame.h"
#include "Items/GripGuide.h"
#include "Items/Inspector.h"
#include "Items/PaneBackground.h"
#include "Items/PrecisionControl.h"
#include "Items/RadialMenu.h"
#include "Items/RadialMenuInfo.h"
#include "Items/SessionState.h"
#include "Items/Settings.h"
#include "Items/Shelf.h"
#include "Items/UnitConversion.h"
#include "Parser/Registry.h"

#define REGISTER_TYPE_(T) Parser::Registry::AddType<T>(#T)

void RegisterItemTypes_();

void RegisterItemTypes_() {
    REGISTER_TYPE_(AppInfo);
    REGISTER_TYPE_(Border);
    REGISTER_TYPE_(BuildVolume);
    REGISTER_TYPE_(Controller);
    REGISTER_TYPE_(Frame);
    REGISTER_TYPE_(GripGuide);
    REGISTER_TYPE_(Inspector);
    REGISTER_TYPE_(PaneBackground);
    REGISTER_TYPE_(PrecisionControl);
    REGISTER_TYPE_(RadialMenu);
    REGISTER_TYPE_(RadialMenuInfo);
    REGISTER_TYPE_(SessionState);
    REGISTER_TYPE_(Settings);
    REGISTER_TYPE_(Shelf);
    REGISTER_TYPE_(UnitConversion);
}

#undef REGISTER_TYPE_

