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

    /// Redefines this to also update the transforms in the ConvertedModel or
    /// original Model if necessary.
    virtual void SetStatus(Status status) override;

    /// Sets an offset in local coordinates of the ConvertedModel relative to
    /// the original Model. The offset is the zero vector by default. This can
    /// be used in cases (such as ClippedModel) where the ConvertedModel needs
    /// to have a different translation than the original.
    void SetOffset(const Vector3f &offset) { offset_ = offset; }

    /// Returns the offset last set by SetOffset(). This is the zero vector by
    /// default.
    const Vector3f & GSetOffset() const { return offset_; }

  protected:
    virtual void AddFields() override;
    virtual bool IsValid(std::string &details) override;
    virtual void CreationDone() override;
    virtual TriMesh BuildMesh() override;

    /// Derived classes must implement this to convert the original,
    /// untransformed mesh.
    virtual TriMesh ConvertMesh(const TriMesh &original_mesh) = 0;

    /// Redefines this to copy the original Model.
    virtual void CopyContentsFrom(const Parser::Object &from,
                                  bool is_deep) override;

  private:
    /// \name Parsed fields.
    ///@{
    Parser::ObjectField<Model> original_model_;
    ///@}

    /// Extra offset added to the original Model's translation in object
    /// coordinates for the ConvertedModel.
    Vector3f offset_{0, 0, 0};
};
