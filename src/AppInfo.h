#pragma once

#include <memory>
#include <string>

#include "Parser/Object.h"
#include "SessionState.h"

namespace Parser { class Registry; }

/// The AppInfo class represents application information that is saved with a
/// session to identify versions and session state.
class AppInfo : public Parser::Object {
  public:
    virtual void AddFields() override;
    virtual bool IsValid(std::string &details) override;

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

typedef std::shared_ptr<AppInfo> AppInfoPtr;
