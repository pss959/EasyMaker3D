//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Models/ConvertedModel.h"

#include <ion/math/transformutils.h>

void ConvertedModel::AddFields() {
    AddModelField(operand_model_.Init("operand_model"));

    OperatorModel::AddFields();
}

bool ConvertedModel::IsValid(Str &details) {
    if (! OperatorModel::IsValid(details))
        return false;  // LCOV_EXCL_LINE [cannot happen]
    if (! operand_model_.GetValue()) {
        details = "No operand model specified";
        return false;
    }
    return true;
}

void ConvertedModel::CreationDone() {
    OperatorModel::CreationDone();

    // Add operand model (if it exists) as a child and do not show it by
    // default. No need to do this if this is a clone.
    if (! IsTemplate() && ! IsClone()) {
        if (auto &operand = GetOperandModel()) {
            ASSERT(GetChildModelCount() == 0);
            operand->SetStatus(Status::kAncestorShown);
            OperatorModel::AddChildModel(operand);
        }
    }
}

void ConvertedModel::SetOperandModel(const ModelPtr &model) {
    ASSERT(model);
    if (GetChildModelCount() > 0)
        ClearChildModels();
    OperatorModel::AddChildModel(model);
    operand_model_ = model;
    GetMesh();  // Make sure the center offset is up to date.
    SyncTransformsFromOperand(*model);
}

void ConvertedModel::AddChildModel(const ModelPtr &child) {
    // This should never be called.
    ASSERTM(false, "ConvertedModel::AddChildModel() should not be called");
}

void ConvertedModel::InsertChildModel(size_t index, const ModelPtr &child) {
    // This should never be called.
    ASSERTM(false, "ConvertedModel::InsertChildModel() should not be called");
}

void ConvertedModel::RemoveChildModel(size_t index) {
    // This should never be called.
    ASSERTM(false, "ConvertedModel::RemoveChildModel() should not be called");
}

void ConvertedModel::ReplaceChildModel(size_t index, const ModelPtr &new_child) {
    // This should never be called.
    ASSERTM(false, "ConvertedModel::ReplaceChildModel() should not be called");
}

void ConvertedModel::SetStatus(Status status) {
    const Status prev_status = GetStatus();
    const bool   was_shown   = IsShown();
    ParentModel::SetStatus(status);

    // If the child was shown and now the ConvertedModel is, update the
    // transforms in the ConvertedModel from the operand.
    if (prev_status == Status::kDescendantShown && IsShown()) {
        SyncTransformsFromOperand(*GetOperandModel());
    }

    // If the ConvertedModel was shown and now the child may be, update the
    // transforms in the operand from the ConvertedModel.
    else if (was_shown && status == Status::kDescendantShown) {
        SyncTransformsToOperand(*GetOperandModel());
    }
}

void ConvertedModel::CopyContentsFrom(const Parser::Object &from,
                                      bool is_deep) {
    OperatorModel::CopyContentsFrom(from, is_deep);

    // Clone the operand Model.
    const ConvertedModel &from_cm = static_cast<const ConvertedModel &>(from);
    SetOperandModel(from_cm.GetOperandModel()->CreateClone());
}

void ConvertedModel::SyncTransformsFromOperand(const Model &operand) {
    CopyTransformsFrom(operand);

    // Compensate for the mesh centering offset.
    SetTranslation(GetTranslation() + GetLocalCenterOffset());
}

void ConvertedModel::SyncTransformsToOperand(Model &operand) const {
    operand.CopyTransformsFrom(*this);

    // Compensate for the mesh centering offset.
    operand.SetTranslation(operand.GetTranslation() - GetLocalCenterOffset());
}

TriMesh ConvertedModel::BuildMeshFromOperands() {
    ASSERT(GetOperandModel());
    return ConvertMesh(GetOperandModel()->GetMesh());
}
