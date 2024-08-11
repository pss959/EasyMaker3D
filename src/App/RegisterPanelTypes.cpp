//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Panels/ActionPanel.h"
#include "Panels/BevelToolPanel.h"
#include "Panels/Board.h"
#include "Panels/CSGToolPanel.h"
#include "Panels/DialogPanel.h"
#include "Panels/ExtrudedToolPanel.h"
#include "Panels/FilePanel.h"
#include "Panels/HelpPanel.h"
#include "Panels/ImportToolPanel.h"
#include "Panels/InfoPanel.h"
#include "Panels/KeyboardPanel.h"
#include "Panels/NameToolPanel.h"
#include "Panels/RadialMenuPanel.h"
#include "Panels/RevSurfToolPanel.h"
#include "Panels/SessionPanel.h"
#include "Panels/SettingsPanel.h"
#include "Panels/TaperToolPanel.h"
#include "Panels/TextToolPanel.h"
#include "Panels/TreePanel.h"
#include "Parser/Registry.h"

#define REGISTER_TYPE_(T) Parser::Registry::AddType<T>(#T)

void RegisterPanelTypes_();

void RegisterPanelTypes_() {
    REGISTER_TYPE_(ActionPanel);
    REGISTER_TYPE_(BevelToolPanel);
    REGISTER_TYPE_(Board);
    REGISTER_TYPE_(CSGToolPanel);
    REGISTER_TYPE_(DialogPanel);
    REGISTER_TYPE_(ExtrudedToolPanel);
    REGISTER_TYPE_(FilePanel);
    REGISTER_TYPE_(HelpPanel);
    REGISTER_TYPE_(ImportToolPanel);
    REGISTER_TYPE_(InfoPanel);
    REGISTER_TYPE_(KeyboardPanel);
    REGISTER_TYPE_(NameToolPanel);
    REGISTER_TYPE_(RadialMenuPanel);
    REGISTER_TYPE_(RevSurfToolPanel);
    REGISTER_TYPE_(SessionPanel);
    REGISTER_TYPE_(SettingsPanel);
    REGISTER_TYPE_(TaperToolPanel);
    REGISTER_TYPE_(TextToolPanel);
    REGISTER_TYPE_(TreePanel);
}

#undef REGISTER_TYPE_

