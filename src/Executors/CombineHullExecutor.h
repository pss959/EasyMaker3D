#pragma once

#include "Executors/CombineExecutorBase.h"

/// CombineHullExecutor executes the CombineHullModelCommand.
///
/// \ingroup Executors
class CombineHullExecutor : public CombineExecutorBase {
  public:
    virtual Str GetCommandTypeName() const override {
        return "CombineHullModelCommand";
    }

  protected:
    virtual CombinedModelPtr CreateCombinedModel(Command &command,
                                                 const Str &name) override;
};
