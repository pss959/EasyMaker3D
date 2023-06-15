#pragma once

#include <cstdio>

#include "Util/Assert.h"

/// This namespace contains general utility functions and classes.
namespace Util {

/// This class can be used to temporarily disable \c stdout and \c stderr
/// output. Any code inside its scope will be redirected to /dev/null.
///
/// \ingroup Utility
class OutputMuter {
  public:
    OutputMuter() {
        saved_stdout_ = *stdout;
        saved_stderr_ = *stderr;
#ifdef ION_PLATFORM_WINDOWS
        auto out = fopen("nul", "w");
#else
        auto out = fopen("/dev/null", "w");
#endif
        ASSERT(out);
        *stdout = *out;
        *stderr = *out;
    }
    ~OutputMuter() {
        *stdout = saved_stdout_;
        *stderr = saved_stderr_;
    }
  private:
    FILE saved_stdout_;
    FILE saved_stderr_;
};

}  // namespace Util
