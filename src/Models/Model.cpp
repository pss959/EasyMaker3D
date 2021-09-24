#include "Models/Model.h"

#include "Assert.h"
#include "Math/MeshUtils.h"
#include "SG/Exception.h"
#include "SG/TriMeshShape.h"
#include "SG/Typedefs.h"

// ----------------------------------------------------------------------------
// Model::Shape_ class.
// ----------------------------------------------------------------------------

/// The Model::Shape_ class is a derived SG::TriMeshShape that allows the Model
/// to update the geometry from a TriMesh built by the derived class.
class Model::Shape_ : public SG::TriMeshShape {
  public:
    Shape_() {
        SetTypeName("Model::Shape_");
    }

    /// Updates the mesh in the shape with the given one.
    void UpdateMesh(const TriMesh &mesh) {
        InstallMesh(mesh);
        if (GetIonShape())
            UpdateIonShapeFromTriMesh(mesh, *GetIonShape());
    }

    /// Returns the TriMesh.
    const TriMesh & GetMesh() const { return GetTriMesh(); }

    ion::gfx::ShapePtr CreateSpecificIonShape() {
        return TriMeshToIonShape(GetTriMesh());
    }
};

// ----------------------------------------------------------------------------
// Model class functions.
// ----------------------------------------------------------------------------

void Model::AllFieldsParsed() {
    PushButtonWidget::AllFieldsParsed();

    // Create a Model::Shape_ instance and set it up.
    shape_.reset(new Shape_);
    AddShape(shape_);
}

void Model::SetStatus(Status status) {
    if (status_ != status) {
        status_ = status;

        /* XXXX
           Material mat = UT.GetMaterialFromRenderer(GetComponent<Renderer>());
           if (mat != null)
           mat.SetInt("_IsSelected", IsSelected() ? 1 : 0);
        */

        // Enable or disable rendering and intersection.
        SetEnabled(Flag::kRender,    IsShown());
        SetEnabled(Flag::kIntersect, IsShown());
    }
}

ModelPtr Model::CreateClone() const {
    ModelPtr clone;

    // XXXX Figure this out!!! (if needed)

    return clone;
}

void Model::SetComplexity(float new_complexity) {
    if (CanSetComplexity()) {
        complexity_ = new_complexity;

        // Assume that a change in complexity results in a change to the
        // geometry.
        ProcessChange(SG::Change::kGeometry);
    }
}

void Model::SetColor(const Color &new_color) {
    color_ = new_color;
    if (is_mesh_valid_)
        SetBaseColor(new_color);
    else
        ProcessChange(SG::Change::kAppearance);
}

const TriMesh & Model::GetMesh() const {
    ASSERT(status_ != Status::kDescendantShown);
    RebuildMeshIfStaleAndShown_();
    return shape_->GetMesh();
}

bool Model::IsMeshValid(std::string &reason) {
    // Make sure the mesh is up to date.
    RebuildMeshIfStaleAndShown_();
    reason = is_mesh_valid_ ? "" : reason_for_invalid_mesh_;
    return is_mesh_valid_;
}

void Model::UpdateForRendering() {
    PushButtonWidget::UpdateForRendering();
    RebuildMeshIfStaleAndShown_();
}

void Model::ProcessChange(const SG::Change &change) {
    PushButtonWidget::ProcessChange(change);
    if (change == SG::Change::kGeometry || change == SG::Change::kGraph)
        MarkMeshAsStale(true);
}

void Model::ThrowReadError(const std::string &msg) {
    throw SG::Exception(GetDesc() + ": " + msg);
}

void Model::RebuildMeshIfStaleAndShown_() const {
    ASSERT(shape_);
    if (is_mesh_stale_ && status_ != Status::kDescendantShown)
        const_cast<Model *>(this)->RebuildMesh_();
}

void Model::RebuildMesh_() {
    ASSERT(shape_);
    shape_->UpdateMesh(BuildMesh());
    is_mesh_stale_ = false;
}
