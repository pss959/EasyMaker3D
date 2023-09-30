#pragma once

#include "Executors/ChangePlaneExecutor.h"

/// ChangeClipExecutor executes the ChangeClipCommand.
///
/// \ingroup Executors
class ChangeClipExecutor : public ChangePlaneExecutor {
  public:
    virtual Str GetCommandTypeName() const override {
        return "ChangeClipCommand";
    }

  protected:
    // Required ChangePlaneExecutor functions:
    virtual Plane GetModelPlane(const Model &model) const override;
    virtual void UpdateModel(Model &model,
                             const PlaneData &data) const override;
    virtual void SetModelPlane(Model &model, const Plane &plane) const override;
};
