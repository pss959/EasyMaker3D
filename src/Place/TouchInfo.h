//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include "Math/Types.h"
#include "Util/Memory.h"

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
