#include "AppInfo.h"

#include "Parser/Registry.h"

void AppInfo::AddFields() {
    Parser::Object::AddFields();
    AddField(app_name_);
    AddField(version_);
    AddField(session_state_);
}

bool AppInfo::IsValid(std::string &details) {
    if (! Parser::Object::IsValid(details))
        return false;
    if (app_name_.GetValue().empty()) {
        details = "Missing app_name field data";
        return false;
    }
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
    // TODO: Get real app name and version from SConstruct.
    app_info->app_name_ = "IMakerVR";
    app_info->version_  = "1.0.0";
    app_info->session_state_ = Parser::Registry::CreateObject<SessionState>();
    return app_info;
}
