#pragma once

#include "Executors/CombineExecutorBase.h"

/// CreateHullExecutor executes the CreateHullModelCommand.
///
/// \ingroup Executors
class CreateHullExecutor : public CombineExecutorBase {
  public:
    virtual std::string GetCommandTypeName() const override {
        return "CreateHullModelCommand";
    }

  protected:
    virtual CombinedModelPtr CreateCombinedModel(
        Command &command, const std::string &name) override;
};
