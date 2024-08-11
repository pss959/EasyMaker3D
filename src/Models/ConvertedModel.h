//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include <vector>

#include "Models/OperatorModel.h"
#include "Util/Memory.h"

DECL_SHARED_PTR(ConvertedModel);

/// ConvertedModel is a derived OperatorModel class that has a single child
/// operand Model. The mesh for a ConvertedModel is typically created by
/// applying some sort of operation to the mesh of the operand Model.
///
/// Unlike CombinedModel, a ConvertedModel keeps its coordinate system in sync
/// with the operand Model as much as possible; it would be jarring to the
/// user otherwise. For example, consider converting a rotated BoxModel to a
/// BeveledModel. The user would expect to be able to translate the
/// BeveledModel in the same local coordinates as the BoxModel.
///
/// There may be some exceptions to the transformation sync:
///  - A derived class (such as BeveledModel) may need to apply scaling to the
///    operand Model's mesh before applying its changes, so its scale will
///    differ from the operand's.
///  - The mesh of a derived class (such as ClippedModel) may have a different
///    center than the operand Model's mesh. The translation would be tweaked
///    in this case (using the center offset) to take that into account.
///
/// \ingroup Models
class ConvertedModel : public OperatorModel {
  public:
    /// Sets the operand Model that this one is to convert.
    void SetOperandModel(const ModelPtr &model);

    /// Returns the operand Model.
    const ModelPtr & GetOperandModel() const { return operand_model_; }

    /// Redefines this to assert, since it should never be called.
    virtual void AddChildModel(const ModelPtr &child) override;
    /// Redefines this to assert, since it should never be called.
    virtual void InsertChildModel(size_t index, const ModelPtr &child) override;
    /// Redefines this to assert, since it should never be called.
    virtual void RemoveChildModel(size_t index) override;
    /// Redefines this to assert, since it should never be called.
    virtual void ReplaceChildModel(size_t index,
                                   const ModelPtr &new_child) override;

    /// Redefines this to also update the transforms in the ConvertedModel or
    /// operand Model if necessary.
    virtual void SetStatus(Status status) override;

  protected:
    virtual void AddFields() override;
    virtual bool IsValid(Str &details) override;
    virtual void CreationDone() override;

    /// Redefines this to copy the operand Model.
    virtual void CopyContentsFrom(const Parser::Object &from,
                                  bool is_deep) override;

    /// This is called to synchronize the transforms from the given operand
    /// Model to this one. The base class implements this to just copy all
    /// transforms and then compensate for any mesh offset.
    virtual void SyncTransformsFromOperand(const Model &operand);

    /// This is called to synchronize the transforms to the given operand
    /// Model from this one. The base class implements this to just copy all
    /// transforms after compensating for any mesh offset.
    virtual void SyncTransformsToOperand(Model &operand) const;

    /// Defines this to call GetConvertedMesh() on the derived class.
    virtual TriMesh BuildMeshFromOperands() override;

    /// Derived classes must implement this method to return the converted
    /// TriMesh. It is passed the (untransformed) mesh of the operand Model.
    virtual TriMesh ConvertMesh(const TriMesh &mesh) = 0;

  private:
    /// \name Parsed fields.
    ///@{
    Parser::ObjectField<Model> operand_model_;
    ///@}
};
