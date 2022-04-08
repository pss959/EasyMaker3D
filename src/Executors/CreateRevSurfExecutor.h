#pragma once

#include "Executors/ModelExecutorBase.h"

/// CreateRevSurfExecutor executes the CreateRevSurfModelCommand.
///
/// \ingroup Executors
class CreateRevSurfExecutor : public ModelExecutorBase {
  public:
    virtual std::string GetCommandTypeName() const override {
        return "CreateRevSurfModelCommand";
    }

  protected:
    virtual ModelPtr CreateModel(Command &command) override;
};
