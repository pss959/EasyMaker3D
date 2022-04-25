#pragma once

#include "Commands/Command.h"
#include "Memory.h"
#include "Targets/EdgeTarget.h"

DECL_SHARED_PTR(ChangeEdgeTargetCommand);

/// ChangeEdgeTargetCommand is used to change some aspect of the edge target.
///
/// \ingroup Commands
class ChangeEdgeTargetCommand : public Command {
  public:
    virtual std::string GetDescription() const override;

    /// Returns the old EdgeTarget.
    const EdgeTargetPtr & GetOldTarget() const { return old_target_; }

    /// Returns the new EdgeTarget.
    const EdgeTargetPtr & GetNewTarget() const { return new_target_; }

    /// Sets the old EdgeTarget.
    void SetOldTarget(const EdgeTarget &pt);

    /// Sets the new EdgeTarget.
    void SetNewTarget(const EdgeTarget &pt);

  protected:
    ChangeEdgeTargetCommand() {}

    virtual void AddFields() override;
    virtual bool IsValid(std::string &details) override;
    virtual void CreationDone() override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::ObjectField<EdgeTarget> old_target_{"old_target"};
    Parser::ObjectField<EdgeTarget> new_target_{"new_target"};
    ///@}

    friend class Parser::Registry;
};
