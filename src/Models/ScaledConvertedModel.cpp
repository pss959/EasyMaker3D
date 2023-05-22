#include "Models/ScaledConvertedModel.h"

#include "Util/Assert.h"

void ScaledConvertedModel::SyncTransformsFromOperand(const Model &operand) {
    // Leave the scale alone.
    SetRotation(operand.GetRotation());
    SetTranslation(operand.GetTranslation());
}

void ScaledConvertedModel::SyncTransformsToOperand(Model &operand) const {
    // Leave the scale alone.
    operand.SetRotation(GetRotation());
    operand.SetTranslation(GetTranslation());
}

void ScaledConvertedModel::CopyContentsFrom(const Parser::Object &from,
                                            bool is_deep) {
    ConvertedModel::CopyContentsFrom(from, is_deep);

    // Copy the operand scale.
    const ScaledConvertedModel &from_scm =
        static_cast<const ScaledConvertedModel &>(from);
    operand_scale_ = from_scm.operand_scale_;
}
