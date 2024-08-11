//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include <vector>

/// \file
/// This file contains utility functions for printing and accessing stack
/// traces when something goes wrong.
///
/// \ingroup Utility

namespace Util {

/// Returns a vector of strings representing the stack trace. Lines do not end
/// with newlines.
StrVec GetStackTrace(size_t count = 20);

/// Prints a stack trace to stderr.
void PrintStackTrace(size_t count = 20);

}  // namespace Util
