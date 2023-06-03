#pragma once

#include "Executors/ChangePlaneExecutor.h"

/// ChangeMirrorExecutor executes the ChangeMirrorCommand.
///
/// \ingroup Executors
class ChangeMirrorExecutor : public ChangePlaneExecutor {
  public:
    virtual std::string GetCommandTypeName() const override {
        return "ChangeMirrorCommand";
    }

  protected:
    // Required ChangePlaneExecutor functions:
    virtual Plane GetModelPlane(const Model &model) const override;
    virtual void  SetModelPlane(Model &model, const Plane &plane,
                                const SG::CoordConv *cc) const override;
};
