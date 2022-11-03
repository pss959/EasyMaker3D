#pragma once

#include <vector>

#include "Base/Memory.h"
#include "Models/ParentModel.h"

DECL_SHARED_PTR(CombinedModel);

/// CombinedModel is a derived ParentModel class that has any number of child
/// operand Models whose meshes are combined somehow into a single Mesh.
///
/// The Mesh for a CombinedModel is typically created from the meshes of all of
/// the operand Models, which need to be combined in the same coordinate
/// system. Therefore the operand meshes are first converted into the object
/// coordinates of the CombinedModel before combining. The resulting mesh is
/// centered on the origin and the centering offset is stored.
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

    /// Returns the offset vector used to center the CombinedModel's mesh.
    const Vector3f & GetCenterOffset() const { return center_offset_; }

  protected:
    virtual void AddFields() override;
    virtual bool IsValid(std::string &details) override;
    virtual void CreationDone() override;

    /// Redefines this to also mark the mesh as stale when a child has been
    /// transformed.
    virtual bool ProcessChange(SG::Change change, const Object &obj) override;

    /// Defines this to call GetCombinedMesh() on the derived class and
    /// centering the result.
    virtual TriMesh BuildMesh() override;

    /// Derived classes must implement this method to return the combined
    /// TriMesh in the object coordinates of the CombinedModel. The base class
    /// takes care of centering the resulting mesh.
    virtual TriMesh CombineMeshes(const std::vector<TriMesh> &meshes) = 0;

  private:
    /// \name Parsed fields.
    ///@{
    Parser::ObjectListField<Model> operand_models_;
    ///@}

    /// Offset added to the mesh in object coordinates to center it.
    Vector3f center_offset_{0, 0, 0};
};
