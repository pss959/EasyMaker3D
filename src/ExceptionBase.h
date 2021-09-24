#pragma once

#include <exception>
#include <string>

#include "Util/FilePath.h"
#include "Util/String.h"

/// Abstract base class for exceptions that store a message.
class ExceptionBase : public std::exception {
  public:
    virtual ~ExceptionBase() {}
    const char * what() const throw() override { return msg_.c_str(); }
  protected:
    /// Constructor taking a simple string message.
    ExceptionBase(const std::string &msg) : msg_(msg) {}

    /// Constructor also taking a path to a file in which the error occurred.
    ExceptionBase(const Util::FilePath &path, const std::string &msg) {
        msg_ = "\n" + path.ToString() + ": " + msg;
    }

    /// Constructor also taking a path to a file and line number at which the
    /// error occurred.
    ExceptionBase(const Util::FilePath &path, int line_number,
                  const std::string &msg) {
        msg_ = "\n" + path.ToString() + ':' +
            Util::ToString(line_number) + ": " + msg;
    }

  private:
    std::string msg_;
};
