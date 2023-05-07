#pragma once

#include "Base/Memory.h"
#include "Commands/ConvertCommand.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(ConvertTaperCommand);

/// ConvertTaperCommand is used to convert each selected Model that is not a
/// TaperedModel to a TaperedModel.
///
/// \ingroup Commands
class ConvertTaperCommand : public ConvertCommand {
  public:
    virtual std::string GetDescription() const override;

  protected:
    ConvertTaperCommand() {}

  private:
    friend class Parser::Registry;
};
