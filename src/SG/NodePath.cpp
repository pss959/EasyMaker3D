#include "SG/NodePath.h"

#include "Assert.h"
#include "SG/Node.h"

namespace SG {

NodePath NodePath::GetSubPath(const Node &end_node) const {
    NodePath sub_path;
    for (auto &node: *this) {
        sub_path.push_back(node);
        if (node.get() == &end_node)
            break;
    }
    ASSERT(! sub_path.empty());
    ASSERT(sub_path.back().get() == &end_node);
    return sub_path;
}

Point3f NodePath::ToWorld(const Point3f &local_pt) const {
    // XXXX
    return local_pt;
}

Vector3f NodePath::ToWorld(const Vector3f &local_vec) const {
    // XXXX
    return local_vec;
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
