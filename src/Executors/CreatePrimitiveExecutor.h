#pragma once

#include "Executors/ModelExecutorBase.h"

/// CreatePrimitiveExecutor executes the CreatePrimitiveModelCommand.
///
/// \ingroup Executors
class CreatePrimitiveExecutor : public ModelExecutorBase {
  public:
    virtual Str GetCommandTypeName() const override {
        return "CreatePrimitiveModelCommand";
    }

  protected:
    virtual ModelPtr CreateModel(Command &command) override;
};
