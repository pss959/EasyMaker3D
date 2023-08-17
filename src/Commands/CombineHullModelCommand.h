#pragma once

#include "Base/Memory.h"
#include "Commands/CombineCommand.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(CombineHullModelCommand);

/// CombineHullModelCommand is used to create a HullModel from one or more
/// operand Models.
///
/// \ingroup Commands
class CombineHullModelCommand : public CombineCommand {
  public:
    virtual Str GetDescription() const override;

  protected:
    CombineHullModelCommand() {}

  private:
    friend class Parser::Registry;
};
