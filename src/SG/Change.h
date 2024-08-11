//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

namespace SG {

/// Types of change made within a graph.
///
/// \ingroup SG
enum class Change {
    kAppearance,  ///< Some sort of appearance change, such as color.
    kBounds,      ///< Any change that affects bounds.
    kEnable,      ///< Change to enabled flags.
    kGeometry,    ///< Change to geometry, such as complexity or structure.
    kGraph,       ///< Change to graph structure, such as adding a child.
    kTransform,   ///< Change to the transform.
};

}  // namespace SG
