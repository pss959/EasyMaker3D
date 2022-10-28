#pragma once

#include <vector>

#include "Base/Memory.h"
#include "Models/ParentModel.h"

DECL_SHARED_PTR(ConvertedModel);

/// ConvertedModel is a derived ParentModel class that has a single child Model
/// (the "original" Model). Each derived ConvertedModel allows some sort of
/// operation to be applied to the resulting converted Model.
///
/// Unlike CombinedModel, a ConvertedModel uses the same coordinate system as
/// the original Model; it keeps its scale, rotation, and translation in sync
/// with those of the original. This means that changes to either the original
/// Model or the ConvertedModel are propagated to the other. Note that because
/// the meshes of the original and ConvertedModel may have different centers,
/// so the translation propagation has to take that into account.
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

    /// Redefines this to copy the original Model.
    virtual void CopyContentsFrom(const Parser::Object &from,
                                  bool is_deep) override;

  private:
    /// \name Parsed fields.
    ///@{
    Parser::ObjectField<Model> original_model_;
    ///@}

    /// Stores the original Model in original_model_ and updates transforms
    /// based on it.
    void UpdateOriginalModel_(const ModelPtr &model);
};
