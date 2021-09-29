#include "Models/RootModel.h"

void RootModel::UpdateAddedChildModel(Model &child) {
    ParentModel::UpdateAddedChildModel(child);
    child.SetUse(Model::Use::kInScene);
    child.SetStatus(Model::Status::kUnselected);
}
