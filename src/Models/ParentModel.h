#pragma once

#include "Models/Model.h"

/// ParentModel is an abstract derived Model class that maintains a set of
/// child Models.
///
/// \ingroup Models
class ParentModel : public Model {
  public:
    /// Redefines this to alsp update the level of the child Models.
    virtual void SetLevel(int level) override;

    /// Redefines this to also recurse on the child Models.
    virtual void SetUse(Use use) override;

    /// Redefines this to update the child Models and update visibility in some
    /// cases.
    virtual void SetStatus(Status status) override;

    /// \name Child Model Query and Modification Functions.
    /// Note that these are equivalent to the regular child functions in the
    /// SG::Node class, but require all children to be Model instances.
    ///@{

    /// Returns the number of child Models.
    size_t GetChildModelCount() const { return GetChildren().size(); }

    /// Returns the index of the given child Model, or -1 if it is not found.
    int GetChildModelIndex(const ModelPtr &child) const;

    /// Returns the indexed child Model, or null if the index is bad.
    ModelPtr GetChildModel(size_t index) const;

    /// Adds a child Model at the end.
    virtual void AddChildModel(const ModelPtr &child);

    /// Inserts a child Model at the given index.
    virtual void InsertChildModel(size_t index, const ModelPtr &child);

    /// Removes the child Model at the given index. Asserts if the index is
    /// bad.
    virtual void RemoveChildModel(size_t index);

    /// Replaces the child Model at the given index. Asserts if the index is
    /// bad.
    virtual void ReplaceChildModel(size_t index, const ModelPtr &new_child);

    /// Removes all child Models.
    virtual void ClearChildModels();

    ///@}

  protected:
    /// Updates when child visibility changes. The default implementation saves
    /// and resets the current scale so it is not applied to the children when
    /// they are visible.
    virtual void ShowChildren(bool children_shown);

    /// Returns the Status to use for children if it can be inferred from the
    /// parent's status. Otherwise, just returns the parent's status.
    static Status GetChildStatus(Status parentStatus);

    /// Returns a vector containing the meshes of all child Models. Each is
    /// transformed into the local coordinates of the parent.
    std::vector<TriMesh> GetChildMeshes();

    /// Sets up a Model that has been added as a child of this.
    virtual void UpdateAddedChildModel(Model &child);

    /// Sets up a Model that has been removed as a child of this.
    virtual void UpdateRemovedChildModel(Model &child);

  private:
    /// Saves the current scale while children are visible, since the scale
    /// should not be applied to them during that time.
    Vector3f saved_scale_{ 1, 1, 1 };
};

typedef std::shared_ptr<ParentModel> ParentModelPtr;
