#pragma once

#include <vector>

#include "Models/ParentModel.h"

/// CombinedModel is a derived ParentModel class that has any number of child
/// operand Models whose meshes are combined somehow into a single Mesh.
///
/// The Mesh for a CombinedModel is typically created from the meshes of all of
/// the operand Models, which need to be combined in the same coordinate
/// system. Therefore the operand meshes are first converted into the local
/// coordinates of the CombinedModel before combining.
///
/// \ingroup Models
class CombinedModel : public ParentModel {
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
    virtual void CreationDone() override;

  private:
    /// \name Parsed fields.
    ///@{
    Parser::ObjectListField<Model> operand_models_{"operand_models"};
    ///@}
};

typedef std::shared_ptr<CombinedModel> CombinedModelPtr;
