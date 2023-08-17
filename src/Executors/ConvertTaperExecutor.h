#pragma once

#include "Executors/ConvertExecutorBase.h"

/// ConvertTaperExecutor executes the ConvertTaperCommand.
///
/// \ingroup Executors
class ConvertTaperExecutor : public ConvertExecutorBase {
  public:
    virtual Str GetCommandTypeName() const override {
        return "ConvertTaperCommand";
    }

  protected:
    virtual ConvertedModelPtr CreateConvertedModel(const Str &name) override;
};
