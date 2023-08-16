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
    virtual std::string GetDescription() const override;

  protected:
    CombineHullModelCommand() {}

    virtual bool IsValid(std::string &details) override;

  private:
    friend class Parser::Registry;
};
