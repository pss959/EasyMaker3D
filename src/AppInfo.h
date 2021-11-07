#pragma once

#include <memory>
#include <string>

#include "Parser/Object.h"

namespace Parser { class Registry; }

/// The AppInfo class represents application information that is saved with a
/// session to identify versions and session state.
class AppInfo : public Parser::Object {
  public:
    /// SessionState represents the transient state of various toggles set
    /// during the session, allowing them to be saved to and read from session
    /// files.
    class SessionState : public Parser::Object {
      private:
        /// \name Parsed Fields
        ///@{
        Parser::TField<bool> point_target_visible_{"point_target_visible"};
        Parser::TField<bool> edge_target_visible_{"edge_target_visible"};
        Parser::TField<bool> edges_shown_{"edges_shown"};
        Parser::TField<bool> build_volume_visible_{"build_volume_visible"};
        Parser::TField<bool> axis_aligned_{"axis_aligned"};
        ///@}

        friend class Parser::Registry;
    };

    virtual void AddFields() override;

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
