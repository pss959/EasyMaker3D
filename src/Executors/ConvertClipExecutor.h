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

    /// Redefines this to use the primary ClippedModel's plane for all
    /// secondary selections.
    virtual void InitConvertedModel(ConvertedModel &model,
                                    const SelPath &path,
                                    const SelPath &primary_path) override;
};
