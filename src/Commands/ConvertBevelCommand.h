#pragma once

#include "Commands/ConvertCommand.h"
#include "Util/Memory.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(ConvertBevelCommand);

/// ConvertBevelCommand is used to convert each selected Model that is not a
/// BeveledModel to a BeveledModel.
///
/// \ingroup Commands
class ConvertBevelCommand : public ConvertCommand {
  public:
    virtual Str GetDescription() const override;

  protected:
    ConvertBevelCommand() {}

  private:
    friend class Parser::Registry;
};
