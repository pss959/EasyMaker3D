#include "Util/Assert.h"

#if ! defined(RELEASE_BUILD)

#include "Util/String.h"

std::string AssertException::BuildMessage_(const std::string &expr,
                                           const std::source_location &loc,
                                           const std::string &msg) {
    std::string message = std::string(loc.file_name()) + ":" +
        Util::ToString(loc.line()) + ": Assertion failed: " + expr;
    if (! msg.empty())
        message += ": " + msg;
    return message;
}                                                             // LCOV_EXCL_LINE

#endif
