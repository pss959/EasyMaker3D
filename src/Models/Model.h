#pragma once

#include <memory>
#include <string>

#include "Defaults.h"
#include "Math/Types.h"
#include "Widgets/PushButtonWidget.h"

//! Forward reference.
class Model;
typedef std::shared_ptr<Model> ModelPtr;

//! Model is an abstract base class for all 3D models created by the
//! application. A Model contains a single Shape that represents it in the 3D
//! scene. The Shape's mesh must be watertight, meaning that there are no gaps
//! at edges. Models are rendered using the Faceted shader.
//!
//! The Model class is derived from PushButtonWidget so that clicking a Model
//! can be used to select or deselect it.
//!
//! \ingroup Models
class Model : public PushButtonWidget {
  public:
    // ------------------------------------------------------------------------
    // Enums and constants.
    // ------------------------------------------------------------------------

    //! Possible uses for an individual Model.
    enum class Use {
        kNew,          //!< Model was just created; it's use is not known yet.
        kInScene,      //!< Appears somewhere in the scene.
        kInClipboard,  //!< Model is a clone made to add to the clipboard.
        kRemoved,      //!< Removed from scene and unattached.
    };

    //! Status of a Model within the scene (accurate only for Use.InScene).
    enum class Status {
        kUnknown,          //!< Can't tell yet (default value).
        kUnselected,       //!< Visible, but not selected.
        kPrimary,          //!< Primary selection.
        kSecondary,        //!< Secondary selection.
        kHiddenByUser,     //!< User has hidden the model.
        kAncestorShown,    //!< Not shown: an ancestor is visible.
        kDescendantShown,  //!< Not shown: at least one descendant is visible.
    };

    // ------------------------------------------------------------------------
    // Basic public interface.
    // ------------------------------------------------------------------------

    //! Allows the name of the Model to be set.
    void SetName(const std::string &new_name) {
        PushButtonWidget::SetName(new_name);
        // XXXX SetTooltipText(new_name);
    }

    //! Sets the level of the Model. It is 0 by default, so only derived
    //! classes that can have child models should need to change it.
    virtual void SetLevel(int level) { level_ = level; }

    //! Returns the level of the Model in the scene.
    int GetLevel() const { return level_; }

    //! Convenience that checks if the level is 0.
    bool IsTopLevel() const { return GetLevel() == 0; }

    //! Sets the current use of the Model. Derived classes should apply this
    //! recursively to all child models, if any.
    virtual void SetUse(Use use) { use_ = use; }

    //! Returns the current use of the Model.
    Use GetUse() const { return use_; }

    //! Sets the current status of the Model. Derived classes can redefine this
    //! to apply any side effects.
    virtual void SetStatus(Status status);

    //! Returns the current status of the Model.
    Status GetStatus() const { return status_; }

    //! Returns true if the Model is shown, meaning it was not hidden either by
    //! the user or because an ancestor or descendant is shown.
    bool IsShown() const {
        return status_ == Status::kUnselected || IsSelected();
    }

    //! Returns true if the Model is selected.
    bool IsSelected() const {
        return status_ == Status::kPrimary || status_ == Status::kSecondary;
    }

    //! For a Model instance that is a clone, this returns the name of the
    //! Model it was cloned from. It is used to create new cloned names so that
    //! the addition of suffixes does not get out of hand. It will be null for
    //! non-clones.
    const string & GetBaseName() const { return base_name_; }

    //! Creates and returns a deep-copy clone of this Model, recursively if
    //! necessary.
    ModelPtr CreateClone() const;

    // ------------------------------------------------------------------------
    // Complexity handling.
    // ------------------------------------------------------------------------

    //! Indicates whether the Model responds to complexity changes. The base
    //! class implements this to always return false.
    virtual bool CanSetComplexity() { return false; }

    //! Returns the current complexity.
    float GetComplexity() const { return complexity_; }

    //! Sets the complexity for the Model to the new value. The base class
    //! implements this to do nothing except set the value in the instance if
    //! the derived class supports it.
    virtual void SetComplexity(float new_complexity);

    //! Returns the current number of triangles in the Model's mesh.
    size_t GetTriangleCount() const { return GetMesh().GetTriangleCount(); }

    // ------------------------------------------------------------------------
    // Color handling.
    // ------------------------------------------------------------------------

    //! Returns the current color for the Model.
    Color GetColor() const { return color_; }

    //! Sets the color for the Model to the new value.
    void SetColor(const Color &new_color);

    // ------------------------------------------------------------------------
    // Mesh query.
    // ------------------------------------------------------------------------

    //! Returns the mesh stored in the Model (in object coordinates). This mesh
    //! may not always be centered on the origin. This may be called as often
    //! as desired; the mesh will be rebuilt only when necessary. This mesh
    //! should not be modified by callers.
    //!
    //! NOTE: This should \em NOT be called when any descendent of the Model is
    //! visible, as it may not be a valid mesh.
    const TriMesh & GetMesh() const;

    //! Returns true if the Mesh in the Model is valid. If not, the reason
    //! string will be set to the reason it is not considered valid.
    bool IsMeshValid(std::string &reason);

    //! Returns true if the Mesh in the Model is stale, meaning that it needs
    //! to be rebuilt.
    bool IsMeshStale() { return is_mesh_stale_; }

    // ------------------------------------------------------------------------
    // Update.
    // ------------------------------------------------------------------------

    //! Redefines this to return the mesh bounds.
    virtual const Bounds & GetBounds() override;

    //! Redefines this to also make sure the mesh is up to data if the Model is
    //! visible,
    virtual void UpdateForRendering() override;

  protected:
    //! The constructor is protected to make this abstract.
    Model() {}

    //! Redefines this to also mark the mesh as stale when appropriate.
    virtual void ProcessChange(const SG::Change &change) override;

    //! Derived classes can call this to indicate that the mesh needs to be
    //! rebuilt by a call to RebuildMesh(). It can also be used to mark the
    //! mesh as not stale in those rare cases (such as cloning) where the
    //! derived class knows that the mesh is ok as is.
    void MarkMeshAsStale(bool is_stale) { is_mesh_stale_ = is_stale; }

    //! Derived classes must implement this to build and return a mesh
    //! representing the Model. It will be called only to create the Mesh for
    //! the first time or after something calls MarkMeshAsStale(true).
    virtual TriMesh BuildMesh() = 0;

  private:
    //! Level in the scene. 0 == top-level. Valid only for Use::kInScene.
    int level_ = 0;

    //! Current use.
    Use use_ = Use::kNew;

    //! Current status of the Model.
    Status status_ = Status::kUnknown;

    //! For a Model instance that is a clone, this is the name of the Model it
    //! was cloned from. It is used to create new cloned names so that the
    //! addition of suffixes does not get out of hand. It will be null for
    //! non-clones.
    std::string base_name_;

    //! Current complexity.
    float complexity_ = Defaults::kModelComplexity;

    //! The mesh used for the Model.
    TriMesh mesh_;

    //! Mesh bounds. These are recomputed whenever the mesh is.
    Bounds bounds_;

    //! Indicates whether the mesh needs to be rebuilt. Derived classes can
    //! call MarkMeshAsStale(true) to set this when something changes that
    //! affects the Mesh.
    bool is_mesh_stale_ = true;

    //! Indicates whether the current Mesh in the Model is valid, according to
    //! CGAL: it must be valid and closed (watertight).
    bool is_mesh_valid_ = false;

    //! If the mesh is valid, this is empty. Otherwise, it contains the reason
    //! it is not considered valid.
    std::string reason_for_invalid_mesh_ = "Never built";

    //! Current color of the Model. If the mesh is invalid, the invalid color
    //! is shown temporarily, but this stores the real color.
    Color color_ = Color::White();

    //! Returns the Mesh in the Model, rebuilding it first only if necessary
    //! and the Model is visible.
    const TriMesh & GetCurrentMesh_() const;

    //! Rebuilds the Mesh in the Model if it is stale and if a descendent of
    // the Model is not visible (which could mean the Transform in the Model is
    // not set up for its Mesh).
    void RebuildMeshIfStaleAndShown_() const;

    //! Rebuilds the mesh and also stores its bounds.
    void RebuildMesh_();
};
