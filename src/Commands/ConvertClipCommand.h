#pragma once

#include "Commands/ConvertCommand.h"
#include "Memory.h"

DECL_SHARED_PTR(ConvertClipCommand);

/// ConvertClipCommand is used to convert each selected Model that is not a
/// ClippedModel to a ClippedModel.
///
/// \ingroup Commands
class ConvertClipCommand : public ConvertCommand {
  public:
    virtual std::string GetDescription() const override;

  protected:
    ConvertClipCommand() {}

  private:
    friend class Parser::Registry;
};
