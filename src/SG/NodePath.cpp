//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "SG/NodePath.h"

#include "SG/Node.h"
#include "Util/Assert.h"
#include "Util/General.h"

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

Str NodePath::ToString(bool add_types) const {
    if (empty())
        return "<EMPTY>";
    Str s = "<";
    for (size_t i = 0; i < size(); ++i) {
        const auto &node = *(*this)[i];
        const Str &name = node.GetName();
        if (i > 0)
            s += '/';
        if (name.empty())
            s += '*';
        else
            s += name;
        if (add_types)
            s += "(" + node.GetTypeName() + ")";
    }
    s += '>';
    return s;
}

// ----------------------------------------------------------------------------
// Path Searching.
// ----------------------------------------------------------------------------

bool NodePath::ContainsNode(const Node &node) const {
    for (auto n: *this) {
        if (n.get() == &node)
            return true;
    }
    return false;
}

NodePtr NodePath::FindNodeUpwards(
    const std::function<bool(const Node &node)> &pred) const {
    for (const auto &p: *this | std::views::reverse) {
        if (pred(*p))
            return p;
    }
    return NodePtr();
}

}  // namespace SG
