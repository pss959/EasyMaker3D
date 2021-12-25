#include "SG/NodePath.h"

#include <ion/math/matrixutils.h>
#include <ion/math/transformutils.h>

#include "SG/Node.h"
#include "Util/Assert.h"

namespace SG {

NodePath NodePath::GetSubPath(const Node &end_node) const {
    NodePath sub_path;
    for (auto &node: *this) {
        sub_path.push_back(node);
        if (node.get() == &end_node)
            break;
    }
    ASSERT(! sub_path.empty());
    ASSERTM(sub_path.back().get() == &end_node,
            "Did not find " + end_node.GetName() + " in " + ToString());
    return sub_path;
}

NodePath NodePath::GetEndSubPath(const Node &start_node) const {
    // Find the start_node in the path. Once it is found, start pushing nodes
    // onto the sub-path.
    NodePath sub_path;
    bool hit_start = false;
    for (auto &node: *this) {
        if (node.get() == &start_node)
            hit_start = true;
        if (hit_start)
            sub_path.push_back(node);
    }
    ASSERTM(! sub_path.empty(),
            "Did not find " + start_node.GetName() + " in " + ToString());
    return sub_path;
}

NodePath NodePath::Stitch(const NodePath &p0, const NodePath &p1) {
    ASSERT(! p0.empty());
    ASSERT(! p1.empty());
    ASSERT(p0.back() == p1.front());
    NodePath stitched = p0;
    Util::AppendVector(std::vector<NodePtr>(p1.begin() + 1, p1.end()),
                       stitched);
    return stitched;
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

// ----------------------------------------------------------------------------
// Local Coordinate Transforms.
// ----------------------------------------------------------------------------

Point3f NodePath::FromLocal(const Point3f &local_pt) const {
    return GetFromLocalMatrix() * local_pt;
}

Vector3f NodePath::FromLocal(const Vector3f &local_vec) const {
    return GetFromLocalMatrix() * local_vec;
}

Point3f NodePath::ToLocal(const Point3f &pt) const {
    return GetToLocalMatrix() * pt;
}

Vector3f NodePath::ToLocal(const Vector3f &vec) const {
    return GetToLocalMatrix() * vec;
}

Matrix4f NodePath::GetFromLocalMatrix() const {
    Matrix4f m = Matrix4f::Identity();
    for (auto &node: *this)
        if (node != back())
            m *= node->GetModelMatrix();
    return m;
}

Matrix4f NodePath::GetToLocalMatrix() const {
    return ion::math::Inverse(GetFromLocalMatrix());
}

// ----------------------------------------------------------------------------
// Object Coordinate Transforms.
// ----------------------------------------------------------------------------

Point3f NodePath::FromObject(const Point3f &object_pt) const {
    return GetFromObjectMatrix() * object_pt;
}

Vector3f NodePath::FromObject(const Vector3f &object_vec) const {
    return GetFromObjectMatrix() * object_vec;
}

Point3f NodePath::ToObject(const Point3f &pt) const {
    return GetToObjectMatrix() * pt;
}

Vector3f NodePath::ToObject(const Vector3f &vec) const {
    return GetToObjectMatrix() * vec;
}

Matrix4f NodePath::GetFromObjectMatrix() const {
    Matrix4f m = Matrix4f::Identity();
    for (auto &node: *this)
        m *= node->GetModelMatrix();
    return m;
}

Matrix4f NodePath::GetToObjectMatrix() const {
    return ion::math::Inverse(GetFromObjectMatrix());
}

// ----------------------------------------------------------------------------
// Path Searching.
// ----------------------------------------------------------------------------

NodePtr NodePath::FindNodeUpwards(
    const std::function<bool(const Node &node)> &pred) const {
    for (auto it = rbegin(); it != rend(); ++it) {
        if (pred(**it))
            return *it;
    }
    return NodePtr();
}

}  // namespace SG
