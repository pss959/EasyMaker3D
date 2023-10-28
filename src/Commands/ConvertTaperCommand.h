#pragma once

#include "Commands/ConvertCommand.h"
#include "Util/Memory.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(ConvertTaperCommand);

/// ConvertTaperCommand is used to convert each selected Model that is not a
/// TaperedModel to a TaperedModel.
///
/// \ingroup Commands
class ConvertTaperCommand : public ConvertCommand {
  public:
    virtual Str GetDescription() const override;

  protected:
    ConvertTaperCommand() {}

  private:
    friend class Parser::Registry;
};
