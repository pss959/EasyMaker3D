//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include <vector>

#include "Util/Memory.h"

namespace Parser {

DECL_SHARED_PTR(Object);
DECL_SHARED_PTR(ObjectList);

/// A ObjectList wraps a vector of pointers to Objects. It has to exist to
/// decouple the Scanner from knowing about the Object class directly.
///
/// \ingroup Parser
struct ObjectList {
    std::vector<ObjectPtr> objects;
};

}  // namespace Parser
