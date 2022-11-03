#pragma once

#include <vector>

#include "Base/Memory.h"
#include "Models/ParentModel.h"

DECL_SHARED_PTR(ConvertedModel);

/// ConvertedModel is a derived ParentModel class that has a single child Model
/// (the "original" Model). Each derived ConvertedModel allows some sort of
/// operation to be applied to the resulting converted Model.
///
/// Unlike CombinedModel, a ConvertedModel keeps its coordinate system in sync
/// with the original Model as much as possible, with some exceptions:
///  - A derived class (such as BeveledModel) may need to apply scaling to the
///    original Model's mesh before applying its changes, so its scale will
///    differ from the original's.
///  - A derived class (such as ClippedModel) may result in an undesired change
///    in position of its mesh relative to the original's, so the translation
///    has to take that into account.
///
/// \ingroup Models
class ConvertedModel : public ParentModel {
  public:
    /// Sets the original Model that this one is to convert.
    void SetOriginalModel(const ModelPtr &model);

    /// Returns the original Model.
    const ModelPtr & GetOriginalModel() const { return original_model_; }

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
    /// original Model if necessary.
    virtual void SetStatus(Status status) override;

  protected:
    virtual void AddFields() override;
    virtual bool IsValid(std::string &details) override;
    virtual void CreationDone() override;

    /// Redefines this to copy the original Model.
    virtual void CopyContentsFrom(const Parser::Object &from,
                                  bool is_deep) override;

    /// This is called to synchronize the transforms from the given original
    /// Model to this one. The base class implements this to just copy all
    /// transforms exactly.
    virtual void SyncTransformsFromOriginal(const Model &original);

    /// This is called to synchronize the transforms to the given original
    /// Model from this one. The base class implements this to just copy all
    /// transforms exactly.
    virtual void SyncTransformsToOriginal(Model &original) const;

  private:
    /// \name Parsed fields.
    ///@{
    Parser::ObjectField<Model> original_model_;
    ///@}
};
