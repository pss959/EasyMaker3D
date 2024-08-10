#pragma once

#include "Commands/ChangeSpinCommand.h"
#include "Util/Memory.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(ChangeTwistCommand);

/// ChangeTwistCommand is used to change the twist applied one or more
/// TwistedModel instances.
///
/// \ingroup Commands
class ChangeTwistCommand : public ChangeSpinCommand {
  public:
    virtual Str GetDescription() const override;

  protected:
    ChangeTwistCommand() {}

    friend class Parser::Registry;
};
