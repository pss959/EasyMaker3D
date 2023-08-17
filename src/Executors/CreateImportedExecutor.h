#pragma once

#include "Executors/ModelExecutorBase.h"

/// CreateImportedExecutor executes the CreateImportedModelCommand.
///
/// \ingroup Executors
class CreateImportedExecutor : public ModelExecutorBase {
  public:
    virtual Str GetCommandTypeName() const override {
        return "CreateImportedModelCommand";
    }

  protected:
    virtual ModelPtr CreateModel(Command &command) override;
};
