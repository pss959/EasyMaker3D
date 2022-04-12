#pragma once

#include "Math/Types.h"
#include "Memory.h"
#include "Models/ParentModel.h"
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
    virtual TriMesh BuildMesh() { return TriMesh(); }

    /// Returns a Notifier that is invoked when any of the following changes is
    /// made to a top-level Model (one of the child Models of the RootModel):
    /// adding a Model, removing a Model, hiding a Model, showing a Model.
    Util::Notifier<> & GetTopLevelChanged() { return top_level_changed_; }

    /// Redefines this to update model use and status for top-level Models.
    virtual void UpdateAddedChildModel(Model &child) override;

    /// Resets to original state (with no child Models).
    void Reset();

    /// Shows or hides edges for all Models.
    void ShowEdges(bool show);

    /// Enables or disables real-time clipping of all selected Models using the
    /// given Plane.
    void EnableClipping(bool enable, const Plane &plane);

    /// Activates or deactivates build volume highlighting. When active,
    /// portions of Models outside the given build volume size are shown in a
    /// special color.
    void ActivateBuildVolume(bool active, const Vector3f &size,
                             const Matrix4f &world_to_stage_matrix);

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
