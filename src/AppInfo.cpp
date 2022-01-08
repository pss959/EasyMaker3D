#include "AppInfo.h"

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
