#pragma once

#include "Executors/CombineExecutorBase.h"

/// CombineHullExecutor executes the CombineHullModelCommand.
///
/// \ingroup Executors
class CombineHullExecutor : public CombineExecutorBase {
  public:
    virtual std::string GetCommandTypeName() const override {
        return "CombineHullModelCommand";
    }

  protected:
    virtual CombinedModelPtr CreateCombinedModel(
        Command &command, const std::string &name) override;
};
