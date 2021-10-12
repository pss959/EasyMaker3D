#include <Util/StackTrace.h>

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
static std::vector<std::string> GetStackTraceLines_() {
    void *array[20];
    size_t size;

    // Get void*'s for all entries on the stack
    size = backtrace(array, 20);

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

void PrintStackTrace() {
    std::vector<std::string> lines = GetStackTraceLines_();
    for (const auto &line: GetStackTraceLines_())
        fprintf(stderr, "%s", line.c_str());
}

std::string GetStackTrace() {
    return Util::JoinStrings(GetStackTraceLines_(), "");
}

}  // namespace Util
