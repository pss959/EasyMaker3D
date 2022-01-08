#pragma once

#include <memory>
#include <string>

#include "Parser/Object.h"

namespace Parser { class Registry; }

/// A SessionState represents the transient state of various toggles set during
/// the session, allowing them to be saved to and read from session files.
class SessionState : public Parser::Object {
  public:
    virtual void AddFields() override;

    /// Copies values from another instance.
    void CopyFrom(const SessionState &from) { CopyContentsFrom(from, false); }

    /// Returns true if the two instances have the same settings.
    bool IsSameAs(const SessionState &other) const;

  protected:
    SessionState() {}

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

typedef std::shared_ptr<SessionState> SessionStatePtr;
