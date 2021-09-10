#include <Util/StackTrace.h>

#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <string>

#include "Util/String.h"

namespace Util {

void PrintStackTrace() {
  void *array[20];
  size_t size;

  // Get void*'s for all entries on the stack
  size = backtrace(array, 20);

  char **syms = backtrace_symbols(array, size);

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
      fprintf(stderr, "[%lu]: %s\n", i, sym.c_str());
  }
}

}  // namespace Util
