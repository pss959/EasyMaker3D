//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Panes/BoxPane.h"
#include "Panes/ButtonPane.h"
#include "Panes/CheckboxPane.h"
#include "Panes/ClipPane.h"
#include "Panes/DropdownPane.h"
#include "Panes/GridPane.h"
#include "Panes/IconPane.h"
#include "Panes/LabeledSliderPane.h"
#include "Panes/ProfilePane.h"
#include "Panes/RadioButtonPane.h"
#include "Panes/ScrollingPane.h"
#include "Panes/SliderPane.h"
#include "Panes/SpacerPane.h"
#include "Panes/SpecialKeyPane.h"
#include "Panes/SwitcherPane.h"
#include "Panes/TaperProfilePane.h"
#include "Panes/TextInputPane.h"
#include "Panes/TextKeyPane.h"
#include "Panes/TextPane.h"
#include "Panes/TouchWrapperPane.h"
#include "Parser/Registry.h"

#define REGISTER_TYPE_(T) Parser::Registry::AddType<T>(#T)

void RegisterPaneTypes_();

void RegisterPaneTypes_() {
    REGISTER_TYPE_(BoxPane);
    REGISTER_TYPE_(ButtonPane);
    REGISTER_TYPE_(CheckboxPane);
    REGISTER_TYPE_(ClipPane);
    REGISTER_TYPE_(DropdownPane);
    REGISTER_TYPE_(GridPane);
    REGISTER_TYPE_(IconPane);
    REGISTER_TYPE_(LabeledSliderPane);
    REGISTER_TYPE_(ProfilePane);
    REGISTER_TYPE_(RadioButtonPane);
    REGISTER_TYPE_(ScrollingPane);
    REGISTER_TYPE_(SliderPane);
    REGISTER_TYPE_(SpacerPane);
    REGISTER_TYPE_(SpecialKeyPane);
    REGISTER_TYPE_(SwitcherPane);
    REGISTER_TYPE_(TaperProfilePane);
    REGISTER_TYPE_(TextInputPane);
    REGISTER_TYPE_(TextKeyPane);
    REGISTER_TYPE_(TextPane);
    REGISTER_TYPE_(TouchWrapperPane);
}

#undef REGISTER_TYPE_

