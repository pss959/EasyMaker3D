#pragma once

#include <cstdio>

/// This namespace contains general utility functions and classes.
namespace Util {

/// This class can be used to temporarily disable \c stdout and \c stderr
/// output. Any code inside its scope will be redirected to /dev/null.
class OutputMuter {
  public:
    OutputMuter() {
        saved_stdout_ = *stdout;
        saved_stderr_ = *stderr;
        *stdout = *fopen("/dev/null", "w");
        *stderr = *fopen("/dev/null", "w");
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
