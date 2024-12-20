//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include <functional>

/// \file
/// This file contains utility functions for processing URLs.
///
/// \ingroup Utility

namespace Util {

/// Sets a function to invoke to simulate opening a URL for testing purposes.
void SetOpenURLFunc(const std::function<void(const Str &)> &func);

/// Opens the given URL in a browser.
void OpenURL(const Str &url);

}  // namespace Util
