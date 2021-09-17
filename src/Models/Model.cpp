#include "Models/Model.h"

#include "Assert.h"
#include "Math/MeshUtils.h"

void Model::SetStatus(Status status) {
    if (status_ != status) {
        status_ = status;

        /* XXXX
           Material mat = UT.GetMaterialFromRenderer(GetComponent<Renderer>());
           if (mat != null)
           mat.SetInt("_IsSelected", IsSelected() ? 1 : 0);
        */

        SetEnabled(Flag::kTraversal, IsShown());
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
    return mesh_;
}

bool Model::IsMeshValid(std::string &reason) {
    // Make sure the mesh is up to date.
    RebuildMeshIfStaleAndShown_();
    reason = is_mesh_valid_ ? "" : reason_for_invalid_mesh_;
    return is_mesh_valid_;
}

const Bounds & Model::GetBounds() {
    RebuildMeshIfStaleAndShown_();
    return bounds_;
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

void Model::RebuildMeshIfStaleAndShown_() const {
    if (is_mesh_stale_ && status_ != Status::kDescendantShown)
        const_cast<Model *>(this)->RebuildMesh_();
}

void Model::RebuildMesh_() {
    mesh_   = BuildMesh();
    bounds_ = ComputeMeshBounds(mesh_);
    is_mesh_stale_ = false;
}
