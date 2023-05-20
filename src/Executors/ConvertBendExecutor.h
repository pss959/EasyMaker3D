#pragma once

#include "Executors/ConvertExecutorBase.h"

/// ConvertBendExecutor executes the ConvertBendCommand.
///
/// \ingroup Executors
class ConvertBendExecutor : public ConvertExecutorBase {
  public:
    virtual std::string GetCommandTypeName() const override {
        return "ConvertBendCommand";
    }

  protected:
    virtual ConvertedModelPtr CreateConvertedModel(
        const std::string &name) override;
};
