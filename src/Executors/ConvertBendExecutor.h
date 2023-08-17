#pragma once

#include "Executors/ConvertExecutorBase.h"

/// ConvertBendExecutor executes the ConvertBendCommand.
///
/// \ingroup Executors
class ConvertBendExecutor : public ConvertExecutorBase {
  public:
    virtual Str GetCommandTypeName() const override {
        return "ConvertBendCommand";
    }

  protected:
    virtual ConvertedModelPtr CreateConvertedModel(const Str &name) override;
};
