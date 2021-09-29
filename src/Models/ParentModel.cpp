#include "Models/ParentModel.h"

#include "Math/MeshUtils.h"
#include "SG/Typedefs.h"
#include "Util/General.h"

void ParentModel::SetLevel(int level) {
    if (level != GetLevel()) {
        Model::SetLevel(level);
        for (size_t i = 0; i < GetChildModelCount(); ++i)
            GetChildModel(i)->SetLevel(level + 1);
    }
}

void ParentModel::SetUse(Use use) {
    Model::SetUse(use);
    for (size_t i = 0; i < GetChildModelCount(); ++i)
        GetChildModel(i)->SetUse(use);
}

void ParentModel::SetStatus(Status status) {
    if (GetStatus() != status) {
        const bool were_children_shown =
            GetStatus() == Status::kDescendantShown;

        // Let the base class update this Model.
        Model::SetStatus(status);

        // Let the derived class handle child visibility changes - they will
        // have to modify the transform data appropriately. XXXX
        const bool are_children_shown = GetStatus() == Status::kDescendantShown;
        if (are_children_shown != were_children_shown)
            ShowChildren(are_children_shown);

        // Update status of child Models.
        const Status child_status = GetChildStatus(status);
        if (child_status != Status::kUnknown)
            for (size_t i = 0; i < GetChildModelCount(); ++i)
                GetChildModel(i)->SetStatus(child_status);
    }
}

int ParentModel::GetChildModelIndex(const ModelPtr &child) const {
    return GetChildIndex(child);
}

ModelPtr ParentModel::GetChildModel(size_t index) const {
    SG::NodePtr child = GetChild(index);
    ASSERT(! child || Util::IsA<Model>(child));
    return Util::CastToDerived<Model>(child);
}

void ParentModel::AddChildModel(const ModelPtr &child) {
    AddChild(child);
    UpdateAddedChildModel(*child);
}

void ParentModel::InsertChildModel(size_t index, const ModelPtr &child) {
    InsertChild(index, child);
    UpdateAddedChildModel(*child);
}

void ParentModel::RemoveChildModel(size_t index) {
    ModelPtr child = GetChildModel(index);
    ASSERT(child);
    UpdateRemovedChildModel(*child);
    RemoveChild(index);
}

void ParentModel::ReplaceChildModel(size_t index, const ModelPtr &new_child) {
    ModelPtr child = GetChildModel(index);
    ASSERT(child);
    UpdateRemovedChildModel(*child);
    ReplaceChild(index, new_child);
}

void ParentModel::ShowChildren(bool children_shown) {
    ASSERT(GetChildModelCount() > 0);

#if XXXX
    // Not sure any of this is necessary...
    // Save the current position.
    // XXXX Vector3f worldPos = transform.position;

    if (children_shown) {
        // Save and reset the scale.
        saved_scale_ = GetScale();
        SetScale(Vector3f(1, 1, 1));
    }
    else {
        // Restore the saved scale.
        SetScale(saved_scale_);
        saved_scale_.Set(1, 1, 1);
    }

    // Restore the position.
    // XXXX transform.position = worldPos;
#endif
}


Model::Status ParentModel::GetChildStatus(Status parent_status) {
    switch (parent_status) {
      case Status::kUnselected:
      case Status::kPrimary:
      case Status::kSecondary:
        // The parent is visible, so the child isn't.
        return Status::kAncestorShown;

      case Status::kDescendantShown:
        // Can't really tell what to do with children - they may be
        // selected. Wait for someone else to set this.
        return Status::kUnknown;

      default:
        // Every other Status gets copied to children.
        return parent_status;
    }
}

std::vector<TriMesh> ParentModel::GetChildMeshes() {
    std::vector<TriMesh> child_meshes;
    for (size_t i = 0; i < GetChildModelCount(); ++i) {
        auto child = GetChildModel(i);
        child_meshes.push_back(TransformMesh(child->GetMesh(),
                                             child->GetModelMatrix()));
    }
    return child_meshes;
}

void ParentModel::UpdateAddedChildModel(Model &child) {
    // One level lower than this one.
    child.SetLevel(GetLevel() + 1);

    // Copy the Use if it is real.
    if (GetUse() != Use::kNew)
        child.SetUse(GetUse());

    child.SetStatus(GetChildStatus(GetStatus()));
}

void ParentModel::UpdateRemovedChildModel(Model &child) {
    child.SetStatus(Status::kUnknown);
    child.SetUse(Use::kRemoved);
    child.SetLevel(-1);
}
