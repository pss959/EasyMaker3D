#pragma once

#include <string>

#include "Commands/CreateModelCommand.h"
#include "Util/Memory.h"

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
