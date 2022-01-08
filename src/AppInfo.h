#pragma once

#include <memory>
#include <string>

#include "Parser/Object.h"
#include "SessionState.h"

namespace Parser { class Registry; }

class AppInfo;
typedef std::shared_ptr<AppInfo> AppInfoPtr;

/// The AppInfo class represents application information that is saved with a
/// session to identify versions and session state.
class AppInfo : public Parser::Object {
  public:
    virtual void AddFields() override;
    virtual bool IsValid(std::string &details) override;

    /// Creates an instance with default values. This uses the current version
    /// of the application.
    static AppInfoPtr CreateDefault();

  protected:
    AppInfo() {}

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<std::string>       app_name_{"app_name"};
    Parser::TField<std::string>       version_{"version"};
    Parser::ObjectField<SessionState> session_state_{"session_state"};
    ///@}

    friend class Parser::Registry;
};
