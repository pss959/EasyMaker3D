#pragma once

#include "Executors/ConvertExecutorBase.h"

/// ConvertTaperExecutor executes the ConvertTaperCommand.
///
/// \ingroup Executors
class ConvertTaperExecutor : public ConvertExecutorBase {
  public:
    virtual std::string GetCommandTypeName() const override {
        return "ConvertTaperCommand";
    }

  protected:
    virtual ConvertedModelPtr CreateConvertedModel(
        const std::string &name) override;
};
