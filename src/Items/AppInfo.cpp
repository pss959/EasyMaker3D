#include "Items/AppInfo.h"

#include "Base/Tuning.h"
 #include "Parser/Registry.h"

void AppInfo::AddFields() {
    AddField(version_.Init("version"));
    AddField(session_state_.Init("session_state"));

    Parser::Object::AddFields();
}

bool AppInfo::IsValid(std::string &details) {
    if (! Parser::Object::IsValid(details))
        return false;
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
