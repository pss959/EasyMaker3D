#pragma once

#include "Executors/ModelExecutorBase.h"

/// CreateTextExecutor executes the CreateTextModelCommand.
///
/// \ingroup Executors
class CreateTextExecutor : public ModelExecutorBase {
  public:
    virtual std::string GetCommandTypeName() const override {
        return "CreateTextModelCommand";
    }

  protected:
    virtual ModelPtr CreateModel(Command &command) override;
};
