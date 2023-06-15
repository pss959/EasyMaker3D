#include "Selection/SelPath.h"

#include <ion/math/matrixutils.h>

#include "Models/Model.h"
#include "Models/RootModel.h"
#include "Util/Assert.h"
#include "Util/General.h"

SelPath::SelPath(const RootModelPtr &root_model, const ModelPtr &child) {
    ASSERTM(root_model->GetChildModelIndex(child) >= 0, child->GetDesc());
    push_back(root_model);
    push_back(child);
}

SelPath::SelPath(const SG::NodePath &node_path) {
    // The path must end at a Model.
    ASSERT(! node_path.empty());
    ASSERT(Util::IsA<Model>(node_path.back()));

    // Find the ModelRoot in the path. Use the sub-path from it to the Model.
    SG::NodePtr root;
    for (auto &node: node_path) {
        if (Util::IsA<RootModel>(node)) {
            root = node;
            break;
        }
    }
    ASSERT(root);
    // Push back all of the nodes in the subpath.
    for (auto &node: node_path.GetEndSubPath(*root))
        push_back(node);
}

ModelPtr SelPath::GetModel() const {
    ASSERT(! empty());
    ModelPtr model = std::dynamic_pointer_cast<Model>(back());
    ASSERT(model);
    return model;
}

ParentModelPtr SelPath::GetParentModel() const {
    ASSERT(size() >= 2U);
    ParentModelPtr parent =
        std::dynamic_pointer_cast<ParentModel>(at(size() - 2));
    ASSERT(parent);
    return parent;
}

void SelPath::Validate() const {
    ASSERT(! empty());
    ASSERT(Util::IsA<RootModel>(front()));
    ASSERT(std::dynamic_pointer_cast<Model>(back()));
}

std::vector<ModelPtr> SelPath::GetAllModels(bool skip_root) const {
    Validate();
    std::vector<ModelPtr> models;
    models.reserve(size());
    for (size_t i = skip_root ? 1 : 0; i < size(); ++i) {
        ModelPtr model = std::dynamic_pointer_cast<Model>((*this)[i]);
        ASSERT(model);
        models.push_back(model);
    }
    return models;
}

bool SelPath::IsAncestorOf(const SelPath &p) const {
    Validate();
    p.Validate();
    // This is an ancestor if all nodes in the path are at the start of P.
    if (front() != p.front() || size() >= p.size())
        return false;
    for (size_t i = 0; i < size(); ++i)
        if ((*this)[i] != p[i])
            return false;
    return true;
}

SG::CoordConv SelPath::GetCoordConv() const {
    Validate();
    return SG::CoordConv(*this);
}

SelPath SelPath::GetPathToChild(const ModelPtr &child) const {
    ASSERT(Util::IsA<ParentModel>(GetModel()));
    ASSERT(std::dynamic_pointer_cast<ParentModel>(
               GetModel())->GetChildModelIndex(child) >= 0);
    SelPath path_to_child = *this;
    path_to_child.push_back(child);
    return path_to_child;
}
