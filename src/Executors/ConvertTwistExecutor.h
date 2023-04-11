#pragma once

#include "Executors/ConvertExecutorBase.h"

/// ConvertTwistExecutor executes the ConvertTwistCommand.
///
/// \ingroup Executors
class ConvertTwistExecutor : public ConvertExecutorBase {
  public:
    virtual std::string GetCommandTypeName() const override {
        return "ConvertTwistCommand";
    }

  protected:
    virtual ConvertedModelPtr CreateConvertedModel() override;
};
