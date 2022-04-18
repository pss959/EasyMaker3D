#pragma once

#include <string>

#include "Commands/CreateModelCommand.h"
#include "Memory.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(CreateRevSurfModelCommand);

/// CreateRevSurfModelCommand is used to create a RevSurfModel.
///
/// \ingroup Commands
class CreateRevSurfModelCommand : public CreateModelCommand {
  public:
    virtual std::string GetDescription() const override;

  protected:
    CreateRevSurfModelCommand() {}

  private:
    friend class Parser::Registry;
};
