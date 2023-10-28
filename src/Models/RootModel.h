#pragma once

#include "Math/Types.h"
#include "Models/ParentModel.h"
#include "Util/Memory.h"
#include "Util/Notifier.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(RootModel);

/// RootModel is a derived ParentModel that is used only as the root containing
/// all Models in the scene. It does not supply a mesh because it is not
/// rendered directly.
///
/// \ingroup Models
class RootModel : public ParentModel {
  public:
    virtual void CreationDone() override;

    virtual TriMesh BuildMesh() { return TriMesh(); }

    /// Returns a Notifier that is invoked when any of the following changes is
    /// made to a top-level Model (one of the child Models of the RootModel):
    /// adding a Model, removing a Model, hiding a Model, showing a Model.
    Util::Notifier<> & GetTopLevelChanged() { return top_level_changed_; }

    /// Redefines this to also update model use and status for top-level Models
    /// and to notify observers.
    virtual void UpdateAddedChildModel(Model &child) override;
    /// Redefines this to also notify observers.
    virtual void UpdateRemovedChildModel(Model &child) override;

    /// Resets to original state (with no child Models).
    void Reset();

    /// \name Uniform processing
    /// Each of these set a uniform value in the RootModel that affects some
    /// aspect of rendering in the Faceted shader.
    ///@{

    /// Updates global uniforms:
    /// - uWorldToStageMatrix: the matrix that converts from world to stage
    ///   coordinates (needed for build volume math).
    /// - uBuildVolumeSize: portions of Models outside this size (in stage
    ///   coordinates) are shown in a special color.
    void UpdateGlobalUniforms(const Matrix4f &wsm, const Vector3f &bv_size);

    /// Shows or hides edges for all Models.
    void ShowEdges(bool show);

    /// Returns true if Model edges are currently shown.
    bool AreEdgesShown() const { return are_edges_shown_; }

    /// \name Model Hiding and Showing.
    ///@{

    /// Hides the given Model, which must be top-level and not hidden.
    void HideModel(const ModelPtr &model);

    /// Shows the given Model, which must be top-level and hidden.
    void ShowModel(const ModelPtr &model);

    /// Returns the current number of hidden top-level Models.
    size_t GetHiddenModelCount();

    /// Hides all top-level Models.
    void HideAllModels();

    /// Shows all hidden Models.
    void ShowAllModels();

    ///@}

  protected:
    RootModel() {}

  private:
    /// Notifies when a change is made to a top-level Model.
    Util::Notifier<> top_level_changed_;

    bool are_edges_shown_ = false;

    friend class Parser::Registry;
};
