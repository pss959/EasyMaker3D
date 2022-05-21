#pragma once

#include <string>

#include "Base/Memory.h"
#include "Parser/Object.h"
#include "Items/SessionState.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(AppInfo);

/// The AppInfo class represents application information that is saved with a
/// session to identify versions and session state.
///
/// \ingroup Items
class AppInfo : public Parser::Object {
  public:
    const std::string     & GetAppName()      const { return app_name_; }
    const std::string     & GetVersion()      const { return version_; }
    const SessionStatePtr & GetSessionState() const { return session_state_; }

    /// Creates an instance with default values. This uses the current version
    /// of the application.
    static AppInfoPtr CreateDefault();

  protected:
    AppInfo() {}

    virtual void AddFields() override;
    virtual bool IsValid(std::string &details) override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<std::string>       app_name_{"app_name"};
    Parser::TField<std::string>       version_{"version"};
    Parser::ObjectField<SessionState> session_state_{"session_state"};
    ///@}

    friend class Parser::Registry;
};
