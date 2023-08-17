#pragma once

#include <string>

#include "Base/Memory.h"
#include "Commands/CreateModelCommand.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(CreateRevSurfModelCommand);

/// CreateRevSurfModelCommand is used to create a RevSurfModel.
///
/// \ingroup Commands
class CreateRevSurfModelCommand : public CreateModelCommand {
  public:
    virtual Str GetDescription() const override;

  protected:
    CreateRevSurfModelCommand() {}

  private:
    friend class Parser::Registry;
};
