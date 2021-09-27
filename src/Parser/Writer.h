#pragma once

#include <ostream>

namespace Parser {

class Object;

/// The Writer class writes any parsed Object to a stream.
class Writer {
  public:
    Writer();
    ~Writer();

    /// Sets a flag indicating whether object addresses should be written as
    /// comments. The default is false.
    void SetAddressFlag(bool write_addresses);

    /// Writes the given Object to a stream.
    void WriteObject(const Object &obj, std::ostream &out);

    private:
    bool write_addresses_ = false;  ///< Whether to write addresses.
};

}  // namespace Parser
