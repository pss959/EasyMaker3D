#pragma once

#include "Executors/ModelExecutorBase.h"

/// CreatePrimitiveExecutor executes the CreatePrimitiveModelCommand.
///
/// \ingroup Executors
class CreatePrimitiveExecutor : public ModelExecutorBase {
  protected:
    virtual ModelPtr CreateModel(Command &command) override;
};
