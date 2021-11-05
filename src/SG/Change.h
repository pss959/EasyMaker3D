#pragma once

namespace SG {

/// Types of change made within a graph.
enum class Change {
    kAppearance,  ///< Some sort of appearance change, such as color.
    kBounds,      ///< Any change that affects bounds.
    kGeometry,    ///< Change to geometry, such as complexity or structure.
    kGraph,       ///< Change to graph structure, such as adding a child.
    kTransform,   ///< Change to the transform.
};

}  // namespace SG
