#pragma once

#include "Models/ConvertedModel.h"
#include "Util/Memory.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(ScaledConvertedModel);

/// ScaledConvertedModel is a derived ConvertedModel class that serves as a
/// base class for Model classes that apply some operation to the scaled
/// version of the operand Model's mesh. The ScaledConvertedModel stores the
/// difference between the operand Model's scale and the ScaledConvertedModel's
/// scale so the two can be kept in sync.
///
/// \ingroup Models
class ScaledConvertedModel : public ConvertedModel {
  protected:
    ScaledConvertedModel() {}
    /// Redefines this to also copy the operand scale.
    virtual void CopyContentsFrom(const Parser::Object &from,
                                  bool is_deep) override;

    /// Overrides this to deal with the difference in scale.
    virtual void SyncTransformsFromOperand(const Model &operand) override;

    /// Overrides this to deal with the difference in scale.
    virtual void SyncTransformsToOperand(Model &operand) const override;

  private:
    /// Scale factors applied to the operand Model's mesh before beveling.
    Vector3f operand_scale_{1, 1, 1};

    friend class Parser::Registry;
};
