#pragma once

#include "Base/Memory.h"
#include "Commands/ConvertCommand.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(ConvertTwistCommand);

/// ConvertTwistCommand is used to convert each selected Model that is not a
/// TwistedModel to a TwistedModel.
///
/// \ingroup Commands
class ConvertTwistCommand : public ConvertCommand {
  public:
    virtual std::string GetDescription() const override;

  protected:
    ConvertTwistCommand() {}

  private:
    friend class Parser::Registry;
};
