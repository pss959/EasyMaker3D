#pragma once

#include <string>

#include "Base/Memory.h"
#include "Commands/CreateModelCommand.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(CreateImportedModelCommand);

/// CreateImportedModelCommand is used to create an ImportedModel.
///
/// \ingroup Commands
class CreateImportedModelCommand : public CreateModelCommand {
  public:
    virtual std::string GetDescription() const override;

  protected:
    CreateImportedModelCommand() {}

  private:
    friend class Parser::Registry;
};
