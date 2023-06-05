#pragma once

#include "Executors/ChangeSpinExecutor.h"

/// ChangeTwistExecutor executes the ChangeTwistCommand.
///
/// \ingroup Executors
class ChangeTwistExecutor : public ChangeSpinExecutor {
  public:
    virtual std::string GetCommandTypeName() const override {
        return "ChangeTwistCommand";
    }

  protected:
    // Required ChangeSpinExecutor functions.
    virtual Spin GetModelSpin(const Model &model) const override;
    virtual void SetModelSpin(Model &model, const Spin &spin) const override;
};
