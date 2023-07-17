#pragma once

#include <exception>
#include <string>
#include <vector>

class FilePath;

/// Abstract base class for exceptions that store a message.
///
/// \ingroup Util
class ExceptionBase : public std::exception {
  public:
    virtual ~ExceptionBase() {}  // LCOV_EXCL_LINE [bug in lcov]
    const char * what() const throw() override { return msg_.c_str(); }
    const std::vector<std::string> & GetStackTrace() const {
        return stack_trace_;
    }

  protected:
    /// Constructor taking a simple string message.
    ExceptionBase(const std::string &msg) : msg_(msg) {
        SetStackTrace_();
    }

    /// Constructor also taking a path to a file in which the error occurred.
    ExceptionBase(const FilePath &path, const std::string &msg);

    /// Constructor also taking a path to a file and line number at which the
    /// error occurred.
    ExceptionBase(const FilePath &path, int line_number,
                  const std::string &msg);

  private:
    std::string              msg_;
    std::vector<std::string> stack_trace_;

    /// Sets the stack trace in the instance.
    void SetStackTrace_();
};
