#pragma once

#include <vector>

#include "Models/OperatorModel.h"
#include "Util/Memory.h"

DECL_SHARED_PTR(CombinedModel);

/// CombinedModel is a derived OperatorModel class that has any number of child
/// operand Models whose meshes are combined somehow into a single mesh.
///
/// The mesh for a CombinedModel is typically created from the meshes of all of
/// the operand Models, which need to be combined in the same coordinate
/// system. Therefore the operand meshes are first converted into the object
/// coordinates of the CombinedModel before combining.
///
/// \ingroup Models
class CombinedModel : public OperatorModel {
  public:
    /// Sets the operand Models that this one is to combine.
    void SetOperandModels(std::vector<ModelPtr> models);

    /// Returns a vector containing all operand Models.
    const std::vector<ModelPtr> & GetOperandModels() const {
        return operand_models_;
    }

    // Redefine all of these to also update the operand_models_ field.
    virtual void AddChildModel(const ModelPtr &child) override;
    virtual void InsertChildModel(size_t index, const ModelPtr &child) override;
    virtual void RemoveChildModel(size_t index) override;
    virtual void ReplaceChildModel(size_t index,
                                   const ModelPtr &new_child) override;

    /// Returns the minimum number of children this CombinedModel must
    /// have. The base class defines this to return 1.
    virtual size_t GetMinChildCount() const { return 1; }

  protected:
    virtual void AddFields() override;
    virtual bool IsValid(Str &details) override;
    virtual void CreationDone() override;

    /// Redefines this to copy the operand Models.
    virtual void CopyContentsFrom(const Parser::Object &from,
                                  bool is_deep) override;

    /// Defines this to call GetCombinedMesh() on the derived class.
    virtual TriMesh BuildMeshFromOperands() override;

    /// Derived classes must implement this method to return the combined
    /// TriMesh. It is passed the meshes of the operand Models already
    /// converted into the object coordinates of the CombinedModel.
    virtual TriMesh CombineMeshes(const std::vector<TriMesh> &meshes) = 0;

  private:
    /// \name Parsed fields.
    ///@{
    Parser::ObjectListField<Model> operand_models_;
    ///@}
};
