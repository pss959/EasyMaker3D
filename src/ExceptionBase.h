#pragma once

#include <exception>
#include <string>

//! Abstract base class for exceptions that store a message.
class ExceptionBase : public std::exception {
  public:
    virtual ~ExceptionBase() {}
    const char * what() const throw() override { return msg_.c_str(); }
  protected:
    ExceptionBase(const std::string &msg) : msg_(msg) {}
  private:
    std::string msg_;
};
