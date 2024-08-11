//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Items/AppInfo.h"

#include "Parser/Registry.h"
#include "Util/Tuning.h"

void AppInfo::AddFields() {
    AddField(version_.Init("version"));
    AddField(session_state_.Init("session_state"));

    Parser::Object::AddFields();
}

bool AppInfo::IsValid(Str &details) {
    if (! Parser::Object::IsValid(details))
        return false;  // LCOV_EXCL_LINE [cannot happen]
    if (version_.GetValue().empty()) {
        details = "Missing version field data";
        return false;
    }
    if (! session_state_.GetValue()) {
        details = "Missing session_state data";
        return false;
    }
    return true;
}

AppInfoPtr AppInfo::CreateDefault() {
    AppInfoPtr app_info = Parser::Registry::CreateObject<AppInfo>();
    app_info->version_  = TK::kVersionString;
    app_info->session_state_ = Parser::Registry::CreateObject<SessionState>();
    return app_info;
}
