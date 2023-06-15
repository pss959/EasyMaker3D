#include "Models/ParentModel.h"

#include "Math/MeshUtils.h"
#include "Util/General.h"
#include "Util/KLog.h"

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
        // Let the base class update this Model.
        Model::SetStatus(status);

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
    return std::dynamic_pointer_cast<Model>(child);
}

void ParentModel::AddChildModel(const ModelPtr &child) {
    KLOG('M', "Adding " << child->GetDesc() << " to " << GetDesc());
    AddChild(child);
    UpdateAddedChildModel(*child);
}

void ParentModel::InsertChildModel(size_t index, const ModelPtr &child) {
    KLOG('M', "Inserting " << child->GetDesc() << " at index " << index
         << " in " << GetDesc());
    InsertChild(index, child);
    UpdateAddedChildModel(*child);
}

void ParentModel::RemoveChildModel(size_t index) {
    ModelPtr child = GetChildModel(index);
    ASSERT(child);
    KLOG('M', "Removing " << child->GetDesc() << " at index " << index
         << " from " << GetDesc());
    UpdateRemovedChildModel(*child);
    RemoveChild(index);
}

void ParentModel::ReplaceChildModel(size_t index, const ModelPtr &new_child) {
    ModelPtr child = GetChildModel(index);
    ASSERT(child);
    KLOG('M', "Replacing " << child->GetDesc() << " at index " << index
         << " with " << new_child->GetDesc() << " in " << GetDesc());
    UpdateRemovedChildModel(*child);
    ReplaceChild(index, new_child);
    UpdateAddedChildModel(*new_child);
}

void ParentModel::ClearChildModels() {
    KLOG('M', "Clearing " << GetChildModelCount() << " children "
         << " from " << GetDesc());
    for (size_t i = 0; i < GetChildModelCount(); ++i)
        UpdateRemovedChildModel(*GetChildModel(i));
    ClearChildren();
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

void ParentModel::CopyContentsFrom(const Parser::Object &from,
                                   bool is_deep) {
    Model::CopyContentsFrom(from, is_deep);

    // Clone all children.
    const ParentModel &from_parent = static_cast<const ParentModel &>(from);
    for (size_t i = 0; i < from_parent.GetChildModelCount(); ++i)
        AddChildModel(from_parent.GetChildModel(i)->CreateClone());
}
