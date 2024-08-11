//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Util/Assert.h"

#if ! defined(RELEASE_BUILD)

#include "Util/String.h"

Str AssertException::BuildMessage_(const Str &expr,
                                   const std::source_location &loc,
                                   const Str &msg) {
    Str message = Str(loc.file_name()) + ":" +
        Util::ToString(loc.line()) + ": Assertion failed: " + expr;
    if (! msg.empty())
        message += ": " + msg;
    return message;
}

#endif
