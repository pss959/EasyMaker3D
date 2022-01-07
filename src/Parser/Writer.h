#pragma once

#include <functional>
#include <memory>
#include <ostream>
#include <string>

namespace Parser {

class Object;

/// The Writer class writes any parsed Object to a stream.
class Writer {
  public:
    explicit Writer(std::ostream &out);
    ~Writer();

    /// Sets a flag indicating whether object addresses should be written as
    /// comments. The default is false.
    void SetAddressFlag(bool write_addresses);

    /// Writes a string as a comment to the stream.
    void WriteComment(const std::string &comment);

    /// Writes the given Object to the stream.
    void WriteObject(const Object &obj);

    /// Writes the given Object to a stream if the given function returns true
    /// for it. The same test is applied to all sub-objects.
    void WriteObjectConditional(
        const Object &obj, const std::function<bool(const Object &)> &func);

  private:
    class Impl_;
    std::unique_ptr<Impl_> impl_;
};

}  // namespace Parser
