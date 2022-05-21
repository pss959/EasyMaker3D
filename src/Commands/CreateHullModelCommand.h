#pragma once

#include "Base/Memory.h"
#include "Commands/CombineCommand.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(CreateHullModelCommand);

/// CreateHullModelCommand is used to create a HullModel from one or more
/// operand Models.
///
/// \ingroup Commands
class CreateHullModelCommand : public CombineCommand {
  public:
    virtual std::string GetDescription() const override;

  protected:
    CreateHullModelCommand() {}

    virtual bool IsValid(std::string &details) override;

  private:
    friend class Parser::Registry;
};
