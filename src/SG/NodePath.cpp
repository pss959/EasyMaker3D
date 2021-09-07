#include "SG/NodePath.h"

#include <ion/math/transformutils.h>

#include "Assert.h"
#include "SG/Node.h"

namespace SG {

//! Helper function that computes a local-to-world matrix for a NodePath.
static Matrix4f ComputeMatrix_(const NodePath &path) {
    Matrix4f m = Matrix4f::Identity();
    for (auto &node: path)
        m *= node->GetModelMatrix();
    return m;
}

NodePath NodePath::GetSubPath(const Node &end_node) const {
    NodePath sub_path;
    for (auto &node: *this) {
        sub_path.push_back(node);
        if (node.get() == &end_node)
            break;
    }
    ASSERT(! sub_path.empty());
    if (sub_path.back().get() != &end_node) {
        std::cerr << "XXXX Invalid SubPath " << sub_path.ToString()
                  << " for path " << ToString() << " and node "
                  << end_node.GetName() << "\n";
    }
    ASSERT(sub_path.back().get() == &end_node);
    return sub_path;
}

Point3f NodePath::ToWorld(const Point3f &local_pt) const {
    return ComputeMatrix_(*this) * local_pt;
}

Vector3f NodePath::ToWorld(const Vector3f &local_vec) const {
    return ComputeMatrix_(*this) * local_vec;
}

std::string NodePath::ToString() const {
    if (empty())
        return "<EMPTY>";
    std::string s = "<";
    for (size_t i = 0; i < size(); ++i) {
        if (i > 0)
            s += '/';
        const std::string &name = (*this)[i]->GetName();
        if (name.empty())
            s += '*';
        else
            s += name;
    }
    s += '>';
    return s;
}


}  // namespace SG
