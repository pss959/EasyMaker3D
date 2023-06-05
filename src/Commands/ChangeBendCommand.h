#pragma once

#include "Base/Memory.h"
#include "Commands/ChangeSpinCommand.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(ChangeBendCommand);

/// ChangeBendCommand is used to change the Spin applied one or more BentModel
/// instances. The Spin is always interpreted in object coordinates of each
/// model.
///
/// \ingroup Commands
class ChangeBendCommand : public ChangeSpinCommand {
  public:
    virtual std::string GetDescription() const override;

  protected:
    ChangeBendCommand() {}

    friend class Parser::Registry;
};
