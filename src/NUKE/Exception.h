#pragma once

#include <string>

#include "ExceptionBase.h"
#include "Parser/Object.h"
#include "Util/FilePath.h"

namespace Input {

//! An instance of this Exception class is thrown when any input function
//! fails.
class Exception : public ExceptionBase {
  public:
    Exception(const Util::FilePath &path, const std::string &msg) :
        ExceptionBase(path, "Error reading: " + msg) {}
    Exception(const Parser::Object &obj, const std::string &msg) :
        ExceptionBase(obj.path, obj.line_number, "Error reading: " + msg) {}
};


}  // namespace Input
