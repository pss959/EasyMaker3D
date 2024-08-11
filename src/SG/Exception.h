//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include "Util/ExceptionBase.h"

namespace SG {

/// Exception thrown when any SG function fails.
///
/// \ingroup SG
class Exception : public ExceptionBase {
  public:
    Exception(const Str &msg) : ExceptionBase(msg) {}
};

}  // namespace SG
