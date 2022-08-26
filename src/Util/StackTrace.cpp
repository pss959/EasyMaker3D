#include <Util/StackTrace.h>

#if defined(ION_PLATFORM_WINDOWS)

namespace Util {

// No stack traces on Windows yet.
void PrintStackTrace(size_t count) {
    fprintf(stderr, "*** No stack trace available on Windows yet\n");
}

std::string GetStackTrace(size_t count) {
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

std::vector<std::string> GetStackTrace(size_t count) {
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
        lines.push_back("[" + Util::ToString(i) + "]: " + sym);
    }
    return lines;
}

void PrintStackTrace(size_t count) {
    for (const auto &line: GetStackTrace(count))
        fprintf(stderr, "%s\n", line.c_str());
}

}  // namespace Util

#endif
