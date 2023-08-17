#pragma once

#include <string>

#include "Base/Memory.h"
#include "Commands/CreateModelCommand.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(CreateExtrudedModelCommand);

/// CreateExtrudedModelCommand is used to create a ExtrudedModel.
///
/// \ingroup Commands
class CreateExtrudedModelCommand : public CreateModelCommand {
  public:
    virtual Str GetDescription() const override;

  protected:
    CreateExtrudedModelCommand() {}

  private:
    friend class Parser::Registry;
};
