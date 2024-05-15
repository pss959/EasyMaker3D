#pragma once

#include <exception>
#include <vector>

class FilePath;

/// Abstract base class for exceptions that store a message.
///
/// \ingroup Utility
class ExceptionBase : public std::exception {
  public:
    virtual ~ExceptionBase() {}
    const char * what() const throw() override { return msg_.c_str(); }
    const StrVec & GetStackTrace() const { return stack_trace_; }

  protected:
    /// Constructor taking a simple string message.
    ExceptionBase(const Str &msg) : msg_(msg) { SetStackTrace_(); }

    /// Constructor also taking a path to a file in which the error occurred.
    ExceptionBase(const FilePath &path, const Str &msg);

    /// Constructor also taking a path to a file and line number at which the
    /// error occurred.
    ExceptionBase(const FilePath &path, int line_number, const Str &msg);

  private:
    Str    msg_;
    StrVec stack_trace_;

    /// Sets the stack trace in the instance.
    void SetStackTrace_();
};
