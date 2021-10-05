#pragma once

#include "Executors/ModelExecutorBase.h"

/// CreatePrimitiveExecutor executes the CreatePrimitiveModelCommand.
///
/// \ingroup Executors
class CreatePrimitiveExecutor : public ModelExecutorBase {
  public:
    virtual std::string GetCommandTypeName() const override {
        return "CreatePrimitiveModelCommand";
    }

  protected:
    virtual ModelPtr CreateModel(Command &command) override;
};
