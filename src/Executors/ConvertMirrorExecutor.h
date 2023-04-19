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

    /// Redefines this to use the primary MirroredModel's plane for all
    /// secondary selections.
    virtual void InitConvertedModel(ConvertedModel &model,
                                    const SelPath &path,
                                    const SelPath &primary_path) override;
};
