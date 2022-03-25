#pragma once

#include "Commands/ConvertCommand.h"
#include "Memory.h"

DECL_SHARED_PTR(ConvertBevelCommand);

/// ConvertBevelCommand is used to convert each selected Model that is not a
/// BeveledModel to a BeveledModel.
///
/// \ingroup Commands
class ConvertBevelCommand : public ConvertCommand {
  public:
    virtual std::string GetDescription() const override;

  protected:
    ConvertBevelCommand() {}

  private:
    friend class Parser::Registry;
};

typedef std::shared_ptr<ConvertBevelCommand> ConvertBevelCommandPtr;
