//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include "Util/Memory.h"

DECL_SHARED_PTR(NameAgent);

/// NameAgent is an abstract interface class that manages names to guarantee
/// uniqueness.
///
/// \ingroup Agents
class NameAgent {
  public:
    /// Adds a name, asserting that it is unique.
    virtual void Add(const Str &name) = 0;

    /// Returns true if the given name is known.
    virtual bool Find(const Str &name) const = 0;
};
