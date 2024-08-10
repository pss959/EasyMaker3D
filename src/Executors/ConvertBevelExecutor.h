#pragma once

#include "Executors/ConvertExecutorBase.h"

/// ConvertBevelExecutor executes the ConvertBevelCommand.
///
/// \ingroup Executors
class ConvertBevelExecutor : public ConvertExecutorBase {
  public:
    virtual Str GetCommandTypeName() const override {
        return "ConvertBevelCommand";
    }

  protected:
    virtual ConvertedModelPtr CreateConvertedModel(const Str &name) override;
};
