#pragma once

#include <istream>
#include <string>
#include <unordered_map>
#include <vector>

#include <ion/math/vector.h>

// Include headers to make using the Parser class easier.
#include "Parser/Exception.h"
#include "Parser/Field.h"
#include "Parser/FieldSpec.h"
#include "Parser/Object.h"
#include "Parser/Typedefs.h"
#include "Parser/Value.h"

namespace Parser {

//! Parses a file or stream to produce a semantic-free graph (usually a tree)
//! of Parser::Object instances. Any errors result in a Parser::Exception being
//! thrown.
class Parser {
  public:
    //! The constructor is passed an std::vector of FieldSpec instances that
    //! tell the parser what fields to expect and how to parse them. Throws an
    //! Exception if there is invalid count in any FieldSpec or if there are
    //! conflicts (multiple FieldSpec instances with the same name but
    //! different types or counts).
    Parser(const std::vector<FieldSpec> &field_specs);

    ~Parser();

    //! Parses the contents of the file with the given path, returning the root
    //! Object in the parse graph.
    ObjectPtr ParseFile(const std::string &path);

    //! Parses the contents of the given stream, returning the root Object in
    //! the parse graph.
    ObjectPtr ParseStream(std::istream &in);

  private:
    //! Type for map from field name to FieldSpec.
    typedef std::unordered_map<std::string, const FieldSpec *> FieldMap_;

    //! Field specs passed to the constructor.
    const std::vector<FieldSpec> &field_specs_;

    std::string path_;       //!< Stores the path for errors.
    int         cur_line_;   //!< Current line number.
    FieldMap_   field_map_;  //!< Maps field names to FieldSpec instances.

    //! Builds the FieldMap_ from the FieldSpec instances passed to the
    //! constructor. Throws an Exception if any error or conflict is found.
    void BuildFieldMap_();

    //! \name Parsing Functions
    //! Each of these parses some part of the input from the given stream,
    //! returning the appropriate type. Each throws an Exception if anything
    //! goes wrong.
    //@{

    //! Parses the next Object in the stream.
    ObjectPtr   ParseObject_(std::istream &in);

    //! Parses a collection of Object instances (in square brackets, separated
    //! by commas) from the stream.
    std::vector<ObjectPtr> ParseObjectList_(std::istream &in);

    //! Parses all fields for the current object.
    std::vector<FieldPtr> ParseFields_(std::istream &in);

    //! Parses a Field with a single value, based on the given FieldSpec.
    FieldPtr ParseSingleFieldValue_(std::istream &in, const FieldSpec &spec);

    //! Parses a Field with multiple values, based on the given FieldSpec.
    FieldPtr ParseArrayFieldValue_(std::istream &in, const FieldSpec &spec);

    //! Parses a single Value (std::variant), based on the given FieldSpec.
    Value ParseValue_(std::istream &in, const FieldSpec &spec);

    //! Parses a name, which must consist only of alphanumeric characters or
    //! underscores, and must not start with a numeric character.
    std::string ParseName_(std::istream &in);

    //! Parses a boolean value, which is any case-insensitive version of {
    //! "true', "t", "false", or "f".
    bool ParseBool_(std::istream &in);

    //! Parses an integer value.
    int ParseInteger_(std::istream &in);

    //! Parses a floating-point value.
    float ParseFloat_(std::istream &in);

    //! Parses a double-quoted string. This does not (yet) handle escape
    //! sequences.
    std::string ParseQuotedString_(std::istream &in);

    //! Parses a single character. Throws an Exception if it is not the
    //! expected one.
    void ParseChar_(std::istream &in, char expected_c);

    //! Returns the next character in the stream without using it up.
    char PeekChar_(std::istream &in);

    //! Skips all whitespace, including comments. Looks for newlines,
    //! incrementing the current line counter.
    void SkipWhiteSpace_(std::istream &in);

    //! Returns the FieldSpec for the given name. Throws an Exception if the
    //! name is not known.
    const FieldSpec & GetFieldSpec_(const std::string &name);

    //! Throws an Exception with the given message. Adds the current file path
    //! and line number to the message.
    void Throw_(const std::string &msg);
};

}  // namespace Parser
