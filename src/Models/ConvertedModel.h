#pragma once

#include <vector>

#include "Base/Memory.h"
#include "Models/OperatorModel.h"

DECL_SHARED_PTR(ConvertedModel);

/// ConvertedModel is a derived OperatorModel class that has a single child
/// operand Model. The mesh for a ConvertedModel is typically created by
/// applying some sort of operation to the mesh of the operand Model.
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

  protected:
    virtual void AddFields() override;
    virtual bool IsValid(std::string &details) override;
    virtual void CreationDone() override;

    /// Defines this to call GetConvertedMesh() on the derived class.
    virtual TriMesh BuildMeshFromOperands() override;

    /// Derived classes must implement this method to return the converted
    /// TriMesh. It is passed the mesh of the operand Model already converted
    /// into the object coordinates of the ConvertedModel.
    virtual TriMesh ConvertMesh(const TriMesh &mesh) = 0;

  private:
    /// \name Parsed fields.
    ///@{
    Parser::ObjectField<Model> operand_model_;
    ///@}
};
