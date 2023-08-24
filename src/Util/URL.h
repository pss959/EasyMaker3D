#pragma once

#include <functional>
#include <string>

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
