#pragma once

#include "Executors/ConvertExecutorBase.h"

/// ConvertTwistExecutor executes the ConvertTwistCommand.
///
/// \ingroup Executors
class ConvertTwistExecutor : public ConvertExecutorBase {
  public:
    virtual Str GetCommandTypeName() const override {
        return "ConvertTwistCommand";
    }

  protected:
    virtual ConvertedModelPtr CreateConvertedModel(const Str &name) override;
};
