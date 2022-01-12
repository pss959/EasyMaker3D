#include <Util/StackTrace.h>

#if defined(ION_PLATFORM_WINDOWS)

namespace Util {

// No stack traces on Windows yet.
void PrintStackTrace() {
    fprintf(stderr, "*** No stack trace available on Windows yet\n");
}

std::string GetStackTrace() {
    return "*** No stack trace available on Windows yet";
}

}  // namespace Util

#else

#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <string>
#include <vector>

#include "Util/String.h"

namespace Util {

/// Returns a vector of strings for the lines of the stack trace. The lines end
/// in newlines.
static std::vector<std::string> GetStackTraceLines_(size_t count) {
    void *array[count];
    size_t size;

    // Get void*'s for all entries on the stack
    size = backtrace(array, count);

    char **syms = backtrace_symbols(array, size);

    std::vector<std::string> lines;
    for (size_t i = 0; i < size; ++i) {
        // The symbol from backtrace_symbols() is of this form:
        //   object(name+offset) [address]
        // Demangle the name part.
        std::string sym = syms[i];
        std::string::size_type open  = sym.find('(');
        std::string::size_type plus  = sym.find('+', open);
        if (open != std::string::npos && plus != std::string::npos) {
            const std::string name = sym.substr(open + 1, plus - open - 1);
            sym = Util::ReplaceString(sym, name, Util::Demangle(name));
        }
        lines.push_back("[" + Util::ToString(i) + "]: " + sym + "\n");
    }
    return lines;
}

void PrintStackTrace(size_t count) {
    for (const auto &line: GetStackTraceLines_(count))
        fprintf(stderr, "%s", line.c_str());
}

std::string GetStackTrace(size_t count) {
    return Util::JoinStrings(GetStackTraceLines_(count), "");
}

}  // namespace Util

#endif
