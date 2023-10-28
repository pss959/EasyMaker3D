#pragma once

#include <functional>
#include <memory>
#include <ostream>
#include <string>

namespace Parser {

class Field;
class Object;

/// The Writer class writes any parsed Object to a stream.
///
/// \ingroup Parser
class Writer {
  public:
    using ObjectFunc = std::function<bool(const Object &, bool)>;

    explicit Writer(std::ostream &out);
    ~Writer();

    /// Sets a flag indicating whether object addresses should be written as
    /// comments. The default is false.
    void SetAddressFlag(bool write_addresses);

    /// Writes a string as a comment to the stream.
    void WriteComment(const Str &comment);

    /// Writes the given Object to the stream.
    void WriteObject(const Object &obj);

    /// Same as WriteObject(), but queries a user-defined function to determine
    /// if each Object should be written. The function is called when the
    /// Object is about to be written; the flag passed to the function is true
    /// in that case. If the function returns false, the Object is not
    /// written. Otherwise, it is written and the function is called again with
    /// the flag set to false afterwards.
    void WriteObjectConditional(const Object &obj, const ObjectFunc &func);

    /// Writes a single field.
    void WriteField(const Field &field);

  private:
    class Impl_;
    std::unique_ptr<Impl_> impl_;
};

}  // namespace Parser
