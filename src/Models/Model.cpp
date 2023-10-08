#include "Models/Model.h"

#include <limits>
#include <random>

#include <ion/math/matrixutils.h>
#include <ion/math/transformutils.h>
#include <ion/math/vectorutils.h>

#include "Math/ColorRing.h"
#include "Math/Linear.h"
#include "Math/MeshUtils.h"
#include "Math/MeshValidation.h"
#include "Math/Snap3D.h"
#include "Place/DragInfo.h"
#include "SG/ColorMap.h"
#include "SG/Exception.h"
#include "SG/MutableTriMeshShape.h"
#include "Util/Assert.h"
#include "Util/Enum.h"
#include "Util/KLog.h"
#include "Util/Tuning.h"

// ----------------------------------------------------------------------------
// Model::ColorSet_ class.
// ----------------------------------------------------------------------------

/// The Model::ColorSet_ is used to define a set of N pseudo-random colors to
/// use for Models. All colors are within a set range of saturations and
/// values.
class Model::ColorSet_ {
  public:
    ColorSet_();

    /// Resets to initial conditions.
    void Reset() { index_ = 0; }

    /// Returns the next color to use.
    Color GetNext() {
        const Color &color = colors_[index_];
        index_ = (index_ + 1) % colors_.size();
        return color;
    }

  private:
    /// Vector of pseudo-random colors to use for Models.
    std::vector<Color> colors_;

    /// Current index into colors_ vector.
    size_t index_ = 0;
};

Model::ColorSet_::ColorSet_() {
    // Create a random number generator with a constant seed for repeatability.
    std::default_random_engine            gen(0x12345678);
    std::uniform_real_distribution<float> dist(0, 1);

    // Create the colors, alternating among 5 different hue ranges so that
    // consecutive colors are never too close.
    colors_.reserve(TK::kModelColorCount);
    const int   hue_range_count = TK::kModelHueRangeCount;
    const float hue_range_size  = 1 / static_cast<float>(hue_range_count);
    int hue_count = 0;
    for (int i = 0; i < TK::kModelColorCount; ++i) {
        const float min_hue = hue_range_size * (hue_count % hue_range_count);
        const float max_hue = min_hue + hue_range_size;
        hue_count += 3;  // 0, 3, 1, 4, 2, ...

        const float h = Lerp(dist(gen), min_hue, max_hue);
        const float s = Lerp(dist(gen),
                             TK::kModelMinSaturation, TK::kModelMaxSaturation);
        const float v = Lerp(dist(gen),
                             TK::kModelMinValue, TK::kModelMaxValue);
        colors_.push_back(Color::FromHSV(h, s, v));
    }
}

// ----------------------------------------------------------------------------
// Model functions.
// ----------------------------------------------------------------------------

std::unique_ptr<Model::ColorSet_> Model::color_set_;

Model::Model() : complexity_(TK::kModelComplexity) {
}

Model::~Model() {
    KLOG('M', "Destroyed " << GetDesc());
}

void Model::CreationDone() {
    ClickableWidget::CreationDone();

    if (! IsTemplate()) {
        // There should already be a shape if this is a clone, but not
        // otherwise.
        if (IsClone()) {
            ASSERT(shape_);
        }
        else {
            ASSERT(! shape_);
            // Create a MutableTriMeshShape instance and set it up.
            shape_ = Parser::Registry::CreateObject<SG::MutableTriMeshShape>();
            AddShape(std::dynamic_pointer_cast<SG::Shape>(shape_));
        }

        SetTooltipText(GetName());

        // The status of a new Model instance is Status::kUnknown. Disable the
        // Model until the status is known.
        SetEnabled(false);

        // Initialize color management if not already done.
        if (! color_set_)
            color_set_.reset(new ColorSet_);

        KLOG('M', "Created " << GetDesc());
    }
}

bool Model::IsValidName(const Str &name) {
    return ! name.empty() &&
        ! std::isspace(name.front()) &&
        ! std::isspace(name.back());
}

bool Model::ChangeModelName(const Str &new_name, bool is_user_edit) {
    // Do nothing if trying to override a user edit.
    if (is_user_name_ && ! is_user_edit)
        return false;

    KLOG('M', "Changing name of " << GetDesc() << " to '" << new_name << "'");
    ChangeName(new_name);
    SetTooltipText(new_name);
    is_user_name_ = is_user_edit;
    return true;
}

void Model::SetStatus(Status status) {
    if (status_ != status) {
        KLOG('y', "Changing status of " << GetDesc()
             << " from " << Util::EnumName(status_)
             << " to " << Util::EnumName(status));

        status_ = status;

        // If visible, clear all disabled flags.
        if (IsShown()) {
            SetFlagEnabled(Flag::kTraversal, true);
            SetFlagEnabled(Flag::kRender,    true);
            SetFlagEnabled(Flag::kIntersect, true);
        }

        // If hidden by the user or an ancestor is shown, disable this Model
        // completely.
        else if (status_ == Status::kHiddenByUser ||
            status_ == Status::kAncestorShown) {
            SetFlagEnabled(Flag::kTraversal, false);
        }

        // If a descendent is shown, disable rendering and intersection of this
        // Model, but leave traversal alone so that the descendents are
        // processed.
        else if (status_ == Status::kDescendantShown) {
            SetFlagEnabled(Flag::kTraversal, true);
            SetFlagEnabled(Flag::kRender,    false);
            SetFlagEnabled(Flag::kIntersect, false);
        }

        // Update the selection count.
        if (status_ == Status::kPrimary || status_ == Status::kSecondary)
            ++selection_count_;
    }
}

ModelPtr Model::CreateClone() const {
    // Do a shallow copy - all Model classes should set internals up properly.
    return CloneTyped<Model>(false);
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
        ProcessChange(SG::Change::kGeometry, *this);
    }
}

Color Model::GetNextColor() {
    ASSERT(color_set_);
    return color_set_->GetNext();
}

void Model::ResetColors() {
    if (color_set_)
        color_set_->Reset();
}

void Model::SetColor(const Color &new_color) {
    color_ = new_color;
    if (GetIonNode())
        UpdateColor_();
    ProcessChange(SG::Change::kAppearance, *this);
}

const TriMesh & Model::GetMesh() const {
    RebuildMeshIfStaleAndShown_(true);
    return shape_->GetMesh();
}

const TriMesh & Model::GetCurrentMesh() const {
    return shape_->GetMesh();
}

bool Model::IsMeshValid(Str &reason) const {
    // Make sure the mesh is up to date.
    RebuildMeshIfStaleAndShown_(true);
    reason = is_mesh_valid_ ? "" : reason_for_invalid_mesh_;
    return is_mesh_valid_;
}

Vector3f Model::GetLocalCenterOffset() const {
    // Make sure the mesh and offset are up to date.
    GetMesh();
    return GetModelMatrix() * GetObjectCenterOffset();
}

void Model::PostSetUpIon() {
    ClickableWidget::PostSetUpIon();
    UpdateColor_();
}

void Model::UpdateForRenderPass(const Str &pass_name) {
    ClickableWidget::UpdateForRenderPass(pass_name);
    RebuildMeshIfStaleAndShown_(true);
}

void Model::PlacePointTarget(const DragInfo &info,
                             Point3f &position, Vector3f &direction,
                             Dimensionality &snapped_dims) {
    // Convert the hit point into stage coordinates. All target work is done in
    // stage coordinates because the precision is defined in those coordinates.
    const Matrix4f wsm = info.GetWorldToStageMatrix();
    position  = wsm * info.hit.GetWorldPoint();
    direction = ion::math::Normalized(wsm * info.hit.GetWorldNormal());

    if (info.is_modified_mode)
        PlacePointTargetOnBounds_(info, position, direction, snapped_dims);
    else
        PlacePointTargetOnMesh_(info, position, direction, snapped_dims);
}

void Model::PlaceEdgeTarget(const DragInfo &info, float current_length,
                            Point3f &position0, Point3f &position1) {
    if (info.is_modified_mode)
        PlaceEdgeTargetOnBounds_(info, position0, position1);
    else
        PlaceEdgeTargetOnMesh_(info, position0, position1);
}

void Model::AddModelField(Parser::Field &field) {
    AddField(field);
    model_fields_.push_back(&field);
}

Bounds Model::UpdateBounds() const {
    RebuildMeshIfStaleAndShown_(false);
    return ClickableWidget::UpdateBounds();
}

bool Model::ProcessChange(SG::Change change, const Object &obj) {
    if (! ClickableWidget::ProcessChange(change, obj)) {
        return false;
    }
    else {
        if (change == SG::Change::kGeometry || change == SG::Change::kGraph)
            MarkMeshAsStale();
        return true;
    }
}

bool Model::ValidateMesh(TriMesh &mesh, Str &reason) {
    KLOG('B', GetDesc() << " validating mesh");
    ASSERT(shape_);
    bool is_valid = false;
    switch (ValidateAndRepairTriMesh(mesh)) {
        using enum MeshValidityCode;
      case kValid:            is_valid = true;                      break;
      case kInconsistent:     reason = "Mesh is inconsistent";      break;
      case kNotClosed:        reason = "Mesh is not closed";        break;
      case kSelfIntersecting: reason = "Mesh is self-intersecting"; break;
    }
    return is_valid;
}

void Model::MarkMeshAsStale() {
    if (! is_mesh_stale_) {
        KLOG('B', GetDesc() << " mesh is now stale");
        is_mesh_stale_ = true;
    }
}

void Model::CopyContentsFrom(const Parser::Object &from, bool is_deep) {
    // Do NOT call the base class version; copy only what is necessary.

    const Model &from_model = static_cast<const Model &>(from);
    level_                   = from_model.level_;
    status_                  = from_model.status_;
    complexity_              = from_model.complexity_;
    is_mesh_stale_           = from_model.is_mesh_stale_;
    is_mesh_valid_           = from_model.is_mesh_valid_;
    reason_for_invalid_mesh_ = from_model.reason_for_invalid_mesh_;
    color_                   = from_model.color_;
    is_user_name_            = from_model.is_user_name_;

    SetScale(from_model.GetScale());
    SetRotation(from_model.GetRotation());
    SetTranslation(from_model.GetTranslation());

    // Set up a shape. Note that CopyContentsFrom() is called before
    // CreationDone() for a clone, so the shape is not yet set up.
    ASSERT(! shape_);
    shape_ = from_model.shape_->CloneTyped<SG::MutableTriMeshShape>(false);
    AddShape(shape_);

    // Copy the base name if there is one. Otherwise, just use the name.
    base_name_ = from_model.base_name_.empty() ?
        from_model.GetName() : from_model.base_name_;

    // Copy all of the Model fields specific to the derived class.
    ASSERT(from_model.model_fields_.size() == model_fields_.size());
    for (size_t i = 0; i < from_model.model_fields_.size(); ++i)
        model_fields_[i]->CopyFrom(*from_model.model_fields_[i], is_deep);
}

void Model::RebuildMeshIfStaleAndShown_(bool notify) const {
    ASSERT(shape_);
    if (is_mesh_stale_ && status_ != Status::kDescendantShown) {
        const bool was_mesh_valid = is_mesh_valid_;
        Model &mutable_model = * const_cast<Model *>(this);
        mutable_model.RebuildMesh_(notify);
        if (GetIonNode() && is_mesh_valid_ != was_mesh_valid)
            mutable_model.UpdateColor_();
    }
}

void Model::RebuildMesh_(bool notify) {
    ASSERT(shape_);

    KLOG('B', GetDesc() << " rebuilding mesh");

    // Disable all notification while building and validating the mesh.
    const bool was_notify_enabled = IsNotifyEnabled();
    SetNotifyEnabled(false);

    // Let the derived class build the origin-centered mesh.
    TriMesh mesh = BuildMesh();
    CleanMesh(mesh);
    ASSERTM(AreClose(ComputeMeshBounds(mesh).GetCenter(), Point3f::Zero()),
            GetDesc() + " has noncentered mesh");

    // Validate the mesh, repairing it if necessary and possible, and install
    // the result.
    reason_for_invalid_mesh_.clear();
    is_mesh_valid_ = ValidateMesh(mesh, reason_for_invalid_mesh_);
    shape_->ChangeMesh(mesh);

    // Clear this flag before notifying so the mesh is not rebuilt.
    is_mesh_stale_ = false;

    // Reenable notification and notify if requested.
    SetNotifyEnabled(was_notify_enabled);
    if (notify)
        ProcessChange(SG::Change::kGeometry, *this);

    // Clear this flag again after notifying, which may set it to true.
    is_mesh_stale_ = false;
}

void Model::PlacePointTargetOnBounds_(const DragInfo &info,
                                      Point3f &position, Vector3f &direction,
                                      Dimensionality &snapped_dims) {
    // Get the matrices to convert between object and stage coordinates.
    const Matrix4f osm = info.GetObjectToStageMatrix();
    const Matrix4f som = ion::math::Inverse(osm);

    // The bounds are guaranteed to be axis-aligned only in object or scaled
    // object coordinates, so do the math in object coordinates to determine
    // which face of the bounds was hit and use its normal as the target
    // direction.
    const Bounds       &obj_bounds = GetBounds();
    Point3f            obj_pos     = info.hit.bounds_point;
    const Bounds::Face face        = obj_bounds.GetFaceForPoint(obj_pos);

    // Testing for snapping to the bounds is done most easily in object
    // coordinates (with aligned bounds), so convert the tolerance value from
    // stage coordinates to object coordinates.
    Vector3f object_tolerance;
    for (int dim = 0; dim < 3; ++dim)
        object_tolerance[dim] =
            ion::math::Length(som * GetAxis(dim, TK::kSnapPointTolerance));

    snapped_dims = Snap3D::SnapToBounds(obj_bounds, obj_pos, object_tolerance);

    // Convert the point and normal into stage coordinates.
    position  = osm * obj_pos;
    direction = TransformNormal(obj_bounds.GetFaceNormal(face), osm);
}

void Model::PlacePointTargetOnMesh_(const DragInfo &info,
                                    Point3f &position, Vector3f &direction,
                                    Dimensionality &snapped_dims) {
    const auto &mesh = GetMesh();

    // See if the point is close enough to snap to any vertex of the Mesh. If
    // multiple vertices are close, choose the best one. Do all of this in
    // stage coordinates.
    const Matrix4f osm = info.GetObjectToStageMatrix();
    float min_dist = std::numeric_limits<float>::max();
    bool is_close = false;
    const Point3f target_position = position;
    for (const auto &pt: mesh.points) {
        const Point3f stage_pt = osm * pt;
        const float dist = ion::math::Distance(stage_pt, target_position);
        if (dist < TK::kSnapPointTolerance && dist < min_dist) {
            position = stage_pt;
            min_dist = dist;
            is_close = true;
        }
    }
    if (is_close)
        snapped_dims = Dimensionality("XYZ");
}

void Model::PlaceEdgeTargetOnBounds_(const DragInfo &info,
                                     Point3f &position0, Point3f &position1) {
    // Convert the bounds intersection point into stage coordinates.
    const Matrix4f osm   = info.GetObjectToStageMatrix();
    const Point3f  point = osm * info.hit.bounds_point;

    // Determine which face of the bounds was hit, in stage coordinates.
    const Bounds bounds = TransformBounds(GetBounds(), osm);
    const Bounds::Face face = bounds.GetFaceForPoint(point);

    // Determine the main face dimension (dim0) and the other 2 (dim1, dim2).
    int dim0, dim1, dim2;
    switch (Bounds::GetFaceDim(face)) {
      case 0:  dim0 = 0; dim1 = 1; dim2 = 2; break;  // Left or right.
      case 1:  dim0 = 1; dim1 = 0; dim2 = 2; break;  // Bottom or top.
      default: dim0 = 2; dim1 = 0; dim2 = 1; break;  // Front or back.
    }

    // Compute the start and end positions of the edge target.
    const Point3f &bmin = bounds.GetMinPoint();
    const Point3f &bmax = bounds.GetMaxPoint();
    const float min1 = std::abs(bmin[dim1] - point[dim1]);
    const float max1 = std::abs(bmax[dim1] - point[dim1]);
    const float min2 = std::abs(bmin[dim2] - point[dim2]);
    const float max2 = std::abs(bmax[dim2] - point[dim2]);
    position0.Set(0, 0, 0);
    position1.Set(0, 0, 0);
    position0[dim0] = position1[dim0] =
        Bounds::IsFaceMax(face) ? bmax[dim0] : bmin[dim0];
    if (std::min(min1, max1) <= std::min(min2, max2)) {
        // dim1 is closer edge dimension.
        position0[dim1] = position1[dim1] =
            min1 < max1 ? bmin[dim1] : bmax[dim1];
        position0[dim2] = bmin[dim2];
        position1[dim2] = bmax[dim2];
    }
    else {
        // dim2 is closer edge dimension.
        position0[dim2] = position1[dim2] =
            min2 < max2 ? bmin[dim2] : bmax[dim2];
        position0[dim1] = bmin[dim1];
        position1[dim1] = bmax[dim1];
    }
}

void Model::PlaceEdgeTargetOnMesh_(const DragInfo &info,
                                   Point3f &position0, Point3f &position1) {
    const auto &mesh = GetMesh();
    const auto &hit  = info.hit;

    // Use the barycentric coordinates of the intersection point to determine
    // which is the closest edge. Since the point is inside the triangle, all
    // barycentric coordinates are in (0,1). The one with the smallest value
    // indicates that the edge on the opposite side is the closest edge.
    // Convert the results to stage coordinates.
    const int min_index = GetMinElementIndex(hit.barycentric);
    const Matrix4f osm = info.GetObjectToStageMatrix();
    position0 = osm * mesh.points[hit.indices[(min_index + 1) % 3]];
    position1 = osm * mesh.points[hit.indices[(min_index + 2) % 3]];
}

void Model::UpdateColor_() {
    SetBaseColor(is_mesh_valid_ ? color_ :
                 SG::ColorMap::SGetColor("InvalidMeshColor"));
}
