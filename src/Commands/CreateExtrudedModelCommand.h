#pragma once

#include "Commands/CreateModelCommand.h"
#include "Util/Memory.h"

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
