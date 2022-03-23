#pragma once

#include "Executors/ModelExecutorBase.h"

/// CreateImportedExecutor executes the CreateImportedModelCommand.
///
/// \ingroup Executors
class CreateImportedExecutor : public ModelExecutorBase {
  public:
    virtual std::string GetCommandTypeName() const override {
        return "CreateImportedModelCommand";
    }

  protected:
    virtual ModelPtr CreateModel(Command &command) override;
};
