#pragma once

#include <string>

#include "Items/SessionState.h"
#include "Parser/Object.h"
#include "Util/Memory.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(AppInfo);

/// The AppInfo class represents application information that is saved with a
/// session to identify versions and session state.
///
/// \ingroup Items
class AppInfo : public Parser::Object {
  public:
    const Str             & GetVersion()      const { return version_; }
    const SessionStatePtr & GetSessionState() const { return session_state_; }

    /// Creates an instance with default values. This uses the current version
    /// of the application.
    static AppInfoPtr CreateDefault();

  protected:
    AppInfo() {}

    virtual void AddFields() override;
    virtual bool IsValid(Str &details) override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<Str>               version_;
    Parser::ObjectField<SessionState> session_state_;
    ///@}

    friend class Parser::Registry;
};
