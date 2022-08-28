#include <Util/StackTrace.h>

// Platform-dependent headers.
#if defined(ION_PLATFORM_WINDOWS)
#  include <windows.h>
#  include <DbgHelp.h>
#else
#  include <execinfo.h>
#  include <stdio.h>
#  include <stdlib.h>
#  include <unistd.h>
#endif

#include <string>
#include <vector>

#include "Util/String.h"

namespace Util {

#if defined(ION_PLATFORM_WINDOWS)

// ----------------------------------------------------------------------------
// Windows version.
// ----------------------------------------------------------------------------
std::vector<std::string> GetStackTrace(size_t count) {
    const auto process = GetCurrentProcess();
    const auto thread  = GetCurrentThread();

    CONTEXT context;
    memset(&context, 0, sizeof(CONTEXT));
    context.ContextFlags = CONTEXT_FULL;
    RtlCaptureContext(&context);

    SymInitialize(process, NULL, TRUE);

    const DWORD image = IMAGE_FILE_MACHINE_AMD64;

    STACKFRAME64 frame;
    ZeroMemory(&frame, sizeof(STACKFRAME64));
    frame.AddrPC.Offset    = context.Rip;
    frame.AddrPC.Mode      = AddrModeFlat;
    frame.AddrFrame.Offset = context.Rsp;
    frame.AddrFrame.Mode   = AddrModeFlat;
    frame.AddrStack.Offset = context.Rsp;
    frame.AddrStack.Mode   = AddrModeFlat;


    std::vector<std::string> lines;
    lines.reserve(count);
    for (size_t i = 0; i < count; i++) {
        // This returns false when the end of the stack is reached.
        if (! StackWalk64(image, process, thread, &frame, &context, NULL,
                          SymFunctionTableAccess64, SymGetModuleBase64, NULL))
            break;

        // Skip the first frame, which is always this function.
        if (i == 0)
            continue;

        char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
        const PSYMBOL_INFO symbol = reinterpret_cast<PSYMBOL_INFO>(buffer);
        symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
        symbol->MaxNameLen   = MAX_SYM_NAME;

        DWORD64 displacement = 0;
        std::string sym_name = "???";
        if (SymFromAddr(process, frame.AddrPC.Offset, &displacement, symbol)) {
            // Demangle. For some reason, the leading underscore is missing on
            // Windows, so add it in first.
            sym_name = Util::Demangle(std::string("_") + symbol->Name);
        }
        lines.push_back("[" + Util::ToString(i) + "]: " + sym_name);
    }

    SymCleanup(process);

    return lines;
}

#else

// ----------------------------------------------------------------------------
// Reasonable version.
// ----------------------------------------------------------------------------
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

#endif

void PrintStackTrace(size_t count) {
    for (const auto &line: GetStackTrace(count))
        fprintf(stderr, "%s\n", line.c_str());
}

}  // namespace Util

