#pragma once

#include "Executors/ModelExecutorBase.h"

/// CreateExtrudedExecutor executes the CreateExtrudedModelCommand.
///
/// \ingroup Executors
class CreateExtrudedExecutor : public ModelExecutorBase {
  public:
    virtual Str GetCommandTypeName() const override {
        return "CreateExtrudedModelCommand";
    }

  protected:
    virtual ModelPtr CreateModel(Command &command) override;
};
