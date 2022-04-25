#pragma once

#include <vector>

#include "Memory.h"
#include "Models/ParentModel.h"

DECL_SHARED_PTR(ConvertedModel);

/// ConvertedModel is a derived ParentModel class that has any number of child
/// operand Models whose meshes are converted somehow into a single Mesh.
///
/// The Mesh for a ConvertedModel is typically created from the meshes of all of
/// the operand Models, which need to be converted in the same coordinate
/// system. Therefore the operand meshes are first converted into the local
/// coordinates of the ConvertedModel before combining.
///
/// \ingroup Models
class ConvertedModel : public ParentModel {
  public:
    /// Sets the original Model that this one is to convert.
    void SetOriginalModel(const ModelPtr &model);

    /// Returns the original Model.
    const ModelPtr & GetOriginalModel() const { return original_model_; }

    // Redefine all of these to also update the original_model_ field.
    virtual void AddChildModel(const ModelPtr &child) override;
    virtual void InsertChildModel(size_t index, const ModelPtr &child) override;
    virtual void RemoveChildModel(size_t index) override;
    virtual void ReplaceChildModel(size_t index,
                                   const ModelPtr &new_child) override;

  protected:
    virtual void AddFields() override;
    virtual bool IsValid(std::string &details) override;
    virtual void CreationDone() override;
    virtual TriMesh BuildMesh() override;

    /// Derived classes must implement this to convert the original mesh.
    virtual TriMesh ConvertMesh(const TriMesh &original_mesh) = 0;

    /// Redefines this to also set original_model_.
    virtual void CopyContentsFrom(const Parser::Object &from,
                                  bool is_deep) override;

  private:
    /// \name Parsed fields.
    ///@{
    Parser::ObjectField<Model> original_model_{"original_model"};
    ///@}
};
