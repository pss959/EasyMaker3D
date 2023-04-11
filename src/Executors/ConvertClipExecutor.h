#pragma once

#include "Executors/ConvertExecutorBase.h"

/// ConvertClipExecutor executes the ConvertClipCommand.
///
/// \ingroup Executors
class ConvertClipExecutor : public ConvertExecutorBase {
  public:
    virtual std::string GetCommandTypeName() const override {
        return "ConvertClipCommand";
    }

  protected:
    virtual ConvertedModelPtr CreateConvertedModel() override;
};
