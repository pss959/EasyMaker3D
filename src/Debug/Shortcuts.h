#pragma once

#if ENABLE_DEBUG_FEATURES

#include <string>

namespace Debug {

/// Potentially handles a debugging shortcut, returning true if handled.
bool HandleShortcut(const std::string &str);

}  // namespace Debug

#endif
