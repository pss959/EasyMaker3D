#pragma once

#include "Base/Memory.h"
#include "Math/Types.h"

namespace SG { DECL_SHARED_PTR(Node); }

/// The TouchInfo struct packages up information needed for controller touch
/// interactions.
///
/// \ingroup Place
struct TouchInfo {
    Point3f     position;    ///< Touch sphere position.
    float       radius = 0;  ///< Touch sphere radius.

    /// Node representing the coordinate system of the touch sphere. Any
    /// touched Widget must be found somewhere in the subgraph rooted by this
    /// node.
    SG::NodePtr root_node;
};
