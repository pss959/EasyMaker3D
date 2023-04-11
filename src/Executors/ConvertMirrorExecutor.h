#pragma once

#include "Executors/ConvertExecutorBase.h"

/// ConvertMirrorExecutor executes the ConvertMirrorCommand.
///
/// \ingroup Executors
class ConvertMirrorExecutor : public ConvertExecutorBase {
  public:
    virtual std::string GetCommandTypeName() const override {
        return "ConvertMirrorCommand";
    }

  protected:
    virtual ConvertedModelPtr CreateConvertedModel() override;
};
