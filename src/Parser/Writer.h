#pragma once

#include <ostream>

namespace Parser {

class Object;

//! The Writer class writes any parsed Object to a stream.
class Writer {
  public:
    Writer();
    ~Writer();

    //! Writes the given Object to a stream.
    void WriteObject(const Object &obj, std::ostream &out);
};

}  // namespace Parser
