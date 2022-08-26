#include "Util/Assert.h"

#if ! defined NDEBUG

#include "Util/StackTrace.h"
#include "Util/String.h"

AssertException::AssertException(const std::string &expr,
                                 const std::string &file,
                                 int line, const std::string &msg) {
    msg_ = file + ":" + Util::ToString(line) + ": Assertion failed: " + expr;
    if (! msg.empty())
        msg_ += ": " + msg;
    stack_trace_ = Util::GetStackTrace();
}

const char * AssertException::what() const throw() {
    return msg_.c_str();
}

const std::vector<std::string> & AssertException::GetStackTrace() const {
    return stack_trace_;
}
#endif
