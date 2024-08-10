#pragma once

#include "Commands/Command.h"
#include "Place/PointTarget.h"
#include "Util/Memory.h"

DECL_SHARED_PTR(ChangePointTargetCommand);

/// ChangePointTargetCommand is used to change some aspect of the point target.
///
/// \ingroup Commands
class ChangePointTargetCommand : public Command {
  public:
    virtual Str GetDescription() const override;

    /// Returns the old PointTarget.
    const PointTargetPtr & GetOldTarget() const { return old_target_; }

    /// Returns the new PointTarget.
    const PointTargetPtr & GetNewTarget() const { return new_target_; }

    /// Sets the old PointTarget.
    void SetOldTarget(const PointTarget &pt);

    /// Sets the new PointTarget.
    void SetNewTarget(const PointTarget &pt);

  protected:
    ChangePointTargetCommand() {}

    virtual void AddFields() override;
    virtual bool IsValid(Str &details) override;
    virtual void CreationDone() override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::ObjectField<PointTarget> old_target_;
    Parser::ObjectField<PointTarget> new_target_;
    ///@}

    friend class Parser::Registry;
};
