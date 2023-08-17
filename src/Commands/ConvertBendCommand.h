#pragma once

#include "Base/Memory.h"
#include "Commands/ConvertCommand.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(ConvertBendCommand);

/// ConvertBendCommand is used to convert each selected Model that is not a
/// BentModel to a BentModel.
///
/// \ingroup Commands
class ConvertBendCommand : public ConvertCommand {
  public:
    virtual Str GetDescription() const override;

  protected:
    ConvertBendCommand() {}

  private:
    friend class Parser::Registry;
};
