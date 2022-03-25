#pragma once

#include <string>

#include "Memory.h"
#include "Parser/Object.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(SessionState);

/// A SessionState represents the transient state of various toggles set during
/// the session, allowing them to be saved to and read from session files.
class SessionState : public Parser::Object {
  public:
    /// Copies values from another instance.
    void CopyFrom(const SessionState &from) { CopyContentsFrom(from, false); }

    /// Returns true if the two instances have the same settings.
    bool IsSameAs(const SessionState &other) const;

    /// \name State query
    ///@{
    bool IsPointTargetVisible() const { return point_target_visible_; }
    bool IsEdgeTargetVisible()  const { return edge_target_visible_;  }
    bool AreEdgesShown()        const { return edges_shown_;          }
    bool IsBuildVolumeVisible() const { return build_volume_visible_; }
    bool IsAxisAligned()        const { return axis_aligned_;         }
    ///@}

    /// \name State modification
    ///@{
    void SetPointTargetVisible(bool b) { point_target_visible_ = b; }
    void SetEdgeTargetVisible(bool b)  { edge_target_visible_  = b; }
    void SetEdgesShown(bool b)         { edges_shown_          = b; }
    void SetBuildVolumeVisible(bool b) { build_volume_visible_ = b; }
    void SetAxisAligned(bool b)        { axis_aligned_         = b; }
    ///@}

  protected:
    SessionState() {}

    virtual void AddFields() override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<bool> point_target_visible_{"point_target_visible", false};
    Parser::TField<bool> edge_target_visible_{"edge_target_visible", false};
    Parser::TField<bool> edges_shown_{"edges_shown", false};
    Parser::TField<bool> build_volume_visible_{"build_volume_visible", false};
    Parser::TField<bool> axis_aligned_{"axis_aligned", false};
    ///@}

    friend class Parser::Registry;
};
