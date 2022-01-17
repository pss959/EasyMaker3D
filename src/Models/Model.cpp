#include "Models/Model.h"

#include <limits>

#include <ion/math/transformutils.h>
#include <ion/math/vectorutils.h>

#include "Defaults.h"
#include "DragInfo.h"
#include "Math/MeshUtils.h"
#include "Math/MeshValidation.h"
#include "SG/Exception.h"
#include "SG/TriMeshShape.h"
#include "SG/Typedefs.h"
#include "Util/Assert.h"

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

void Model::CreationDone(bool is_template) {
    PushButtonWidget::CreationDone(is_template);

    if (! is_template) {
        // Create a Model::Shape_ instance and set it up.
        shape_.reset(new Shape_);
        AddShape(shape_);

        // Make sure the Mesh is built.
        GetMesh();
    }
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

void Model::MoveCenterTo(const Point3f &p) {
    const Point3f obj_center = GetScaledBounds().GetCenter();
    SetTranslation(p - GetRotation() * obj_center);
}

void Model::MoveBottomCenterTo(const Point3f &p, const Vector3f &dir) {
    // Rotate to match the target direction.
    const Rotationf rot = Rotationf::RotateInto(Vector3f(0, 1, 0), dir);
    SetRotation(rot);

    // Move the bottom center of the bounds (after rotation) to the target
    // position.
    const Bounds bounds = GetScaledBounds();
    const Point3f bottom_center = bounds.GetFaceCenter(Bounds::Face::kBottom);
    SetTranslation(p - rot * bottom_center);
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

void Model::UpdateForRenderPass(const std::string &pass_name) {
    PushButtonWidget::UpdateForRenderPass(pass_name);
    RebuildMeshIfStaleAndShown_();
    SetBaseColor(is_mesh_valid_ ? color_ : Defaults::kInvalidMeshColor);
}

void Model::PlacePointTarget(const DragInfo &info,
                             Point3f &position, Vector3f &direction,
                             Dimensionality &snapped_dims) {
    // Convert the hit into stage coordinates. All target work is done in stage
    // coordinates because the precision is defined in those coordinates.
    position  = info.path_to_stage.ToLocal(info.hit.GetWorldPoint());
    direction = ion::math::Normalized(
        info.path_to_stage.ToLocal(info.hit.GetWorldNormal()));

    if (info.is_alternate_mode)
        PlacePointTargetOnBounds_(info, position, direction, snapped_dims);
    else
        PlacePointTargetOnMesh_(info, position, direction, snapped_dims);
}

void Model::ProcessChange(SG::Change change) {
    PushButtonWidget::ProcessChange(change);
    if (change == SG::Change::kGeometry || change == SG::Change::kGraph)
        MarkMeshAsStale(true);
}

void Model::RebuildMeshIfStaleAndShown_() const {
    ASSERT(shape_);
    if (is_mesh_stale_ && status_ != Status::kDescendantShown)
        const_cast<Model *>(this)->RebuildMesh_();
}

void Model::RebuildMesh_() {
    ASSERT(shape_);
    shape_->UpdateMesh(BuildMesh());
    ProcessChange(SG::Change::kGeometry);
    is_mesh_stale_ = false;

    // Validate the new mesh.
    is_mesh_valid_ = false;
    reason_for_invalid_mesh_.clear();
    switch (::IsMeshValid(shape_->GetMesh())) {
      case MeshValidityCode::kValid:
        is_mesh_valid_ = true;
        break;
      case MeshValidityCode::kInconsistent:
        reason_for_invalid_mesh_ = "Mesh is inconsistent";
        break;
      case MeshValidityCode::kNotClosed:
        reason_for_invalid_mesh_ = "Mesh is not closed";
        break;
      case MeshValidityCode::kSelfIntersecting:
        reason_for_invalid_mesh_ = "Mesh is self-intersecting";
        break;
    }
}

void Model::PlacePointTargetOnBounds_(const DragInfo &info,
                                      Point3f &position, Vector3f &direction,
                                      Dimensionality &snapped_dims) {
    // std::cerr << "XXXX Model::PlacePointTargetOnBounds_ not done yet\n";

    // XXXX
}

void Model::PlacePointTargetOnMesh_(const DragInfo &info,
                                    Point3f &position, Vector3f &direction,
                                    Dimensionality &snapped_dims) {
    const TriMesh & mesh = GetMesh();

    // See if the point is close enough (within the current precision) to snap
    // to any vertex of the Mesh.  If multiple vertices are close, choose the
    // best one. Do all of this in stage coordinates.
    const Matrix4f to_stage =
        info.hit.path.GetFromObjectMatrix() *
        info.path_to_stage.GetToObjectMatrix();

    float min_dist = std::numeric_limits<float>::max();
    bool is_close = false;
    for (const auto &pt: mesh.points) {
        const Point3f stage_pt = to_stage * pt;
        const float dist = ion::math::Distance(stage_pt, position);
        if (dist < info.linear_precision && dist < min_dist) {
            position = stage_pt;
            min_dist = dist;
            is_close = true;
        }
    }
    if (is_close) {
        snapped_dims = Dimensionality("XYZ");
    }
}
