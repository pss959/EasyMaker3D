#include "Util/Assert.h"

#if ! defined(RELEASE_BUILD)

#include "Util/String.h"

std::string AssertException::BuildMessage_(const std::string &expr,
                                           const std::string &file,
                                           int line, const std::string &msg) {
    std::string message =
        file + ":" + Util::ToString(line) + ": Assertion failed: " + expr;
    if (! msg.empty())
        message += ": " + msg;
    return message;
}

#endif
