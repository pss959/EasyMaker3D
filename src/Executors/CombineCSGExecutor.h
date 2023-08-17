﻿#pragma once

#include "Executors/CombineExecutorBase.h"

/// CombineCSGExecutor executes the CombineCSGModelCommand.
///
/// \ingroup Executors
class CombineCSGExecutor : public CombineExecutorBase {
  public:
    virtual Str GetCommandTypeName() const override {
        return "CombineCSGModelCommand";
    }

  protected:
    virtual CombinedModelPtr CreateCombinedModel(Command &command,
                                                 const Str &name) override;
};
