#pragma once

#include <memory>

#include "Commands/CombineCommand.h"

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

typedef std::shared_ptr<CreateHullModelCommand> CreateHullModelCommandPtr;
