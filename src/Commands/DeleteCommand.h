#pragma once

#include "Commands/MultiModelCommand.h"
#include "Memory.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(DeleteCommand);

/// DeleteCommand deletes one or more Models from the scene.
///
/// \ingroup Commands
class DeleteCommand : public MultiModelCommand {
  public:
    virtual std::string GetDescription() const override;

  protected:
    DeleteCommand() {}

  private:
    friend class Parser::Registry;
};
