#pragma once

#include <istream>
#include <string>
#include <unordered_map>
#include <vector>

#include <filesystem>

#include <ion/math/vector.h>

// Include headers to make using the Parser class easier.
#include "Parser/Exception.h"
#include "Parser/Field.h"
#include "Parser/FieldSpec.h"
#include "Parser/Object.h"
#include "Parser/ObjectSpec.h"
#include "Parser/Typedefs.h"
#include "Parser/Value.h"

namespace Parser {

//! Parses a file or stream to produce a semantic-free graph (usually a tree)
//! of Parser::Object instances. Any errors result in a Parser::Exception being
//! thrown.
class Parser {
  public:
    //! The constructor is passed an std::vector of ObjectSpec instances that
    //! tell the parser what types of objects to expect and how to parse each
    //! one of them. Throws an Exception if any of the following errors are
    //! found in the specs:
    //!   \li Duplicate Object types.
    //!   \li Invalid zero count for any FieldSpec.
    //!   \li Invalid count > 1 for any FieldSpec of type ValueType::kObject or
    //!       ValueType::kObjectList.
    //!   \li Conflicting field specs within an ObjectSpec (multiple FieldSpec
    //!       instances with the same field name).
    Parser(const std::vector<ObjectSpec> &object_specs);

    ~Parser();

    //! Sets a base path that is used for relative paths passed to
    //! ParseFile(). If the path does not begin with a '/', it is assumed to be
    //! relative. If this is never set, all relative paths are assumed to be
    //! relative to the current directory.
    void SetBasePath(const std::string &base_path) {
        base_path_ = base_path;
    }

    //! Parses the contents of the file with the given path, returning the root
    //! Object in the parse graph.
    ObjectPtr ParseFile(const std::filesystem::path &path);

    //! Parses the contents of the given stream, returning the root Object in
    //! the parse graph.
    ObjectPtr ParseStream(std::istream &in);

  private:
    //! Type for map from Object type name to ObjectSpec.
    typedef std::unordered_map<std::string, const ObjectSpec *> ObjectSpecMap_;

    //! Type for map from qualified field name to FieldSpec. The qualified name
    //! is created by joining the Object type name and field name with a '/'
    //! between.
    typedef std::unordered_map<std::string, const FieldSpec *> FieldSpecMap_;

    //! Type for map from Object name to ObjectPtr. This is used for named
    //! Objects and references to them.
    typedef std::unordered_map<std::string, ObjectPtr> ObjectNameMap_;

    //! Object specs passed to the constructor.
    const std::vector<ObjectSpec> &object_specs_;

    //! Base path to use for relative file paths.
    std::filesystem::path base_path_;

    std::string path_;        //!< Input path as a string, for errors.
    int         cur_line_;    //!< Current line number, for errors.

    ObjectSpecMap_ object_spec_map_;  //!< Maps type name to ObjectSpec.
    FieldSpecMap_  field_spec_map_;   //!< Maps field name to FieldSpec.
    ObjectNameMap_ object_name_map_;  //!< Maps object name to ObjectPtr.

    //! Builds object_spec_map_ and field_spec_map_ from the ObjectSpec
    //! instances passed to the constructor. Throws an Exception if any error
    //! is found.
    void BuildSpecMaps_();

    //! \name Parsing Functions
    //! Each of these parses some part of the input from the given stream,
    //! returning the appropriate type. Each throws an Exception if anything
    //! goes wrong.
    //@{

    //! Parses the next Object in the stream.
    ObjectPtr ParseObject_(std::istream &in);

    //! Parses the contents of an included file, returning its Object.
    ObjectPtr ParseIncludedFile_(std::istream &in);

    //! Parses a collection of Object instances (in square brackets, separated
    //! by commas) from the stream.
    std::vector<ObjectPtr> ParseObjectList_(std::istream &in);

    //! Parses all fields for an object with the given ObjectSpec.
    std::vector<FieldPtr> ParseFields_(std::istream &in,
                                       const ObjectSpec &obj_spec);

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

    //! Returns the ObjectSpec for the given name. Throws an Exception if the
    //! name is not known.
    const ObjectSpec & GetObjectSpec_(const std::string &name);

    //! Returns the FieldSpec for the given qualified name. Throws an Exception
    //! if the name is not known.
    const FieldSpec & GetFieldSpec_(const std::string &name);

    //! Returns an ObjectPtr for the named object, which must match the given
    //! ObjectSpec. Throws an Exception if none is found.
    const ObjectPtr & GetObjectByName_(const std::string &name,
                                       const ObjectSpec &spec);

    //! Returns the qualified name for a field, constructed from its Object
    //! type name and the field name.
    static std::string GetQualifiedFieldName_(const std::string &obj_type_name,
                                              const std::string &field_name) {
        return obj_type_name + '/' + field_name;
    }

    //! Returns the qualified name for an object, constructed from its Object
    //! type name and the object name.
    static std::string GetQualifiedObjectName_(const std::string &obj_type_name,
                                               const std::string &obj_name) {
        return obj_type_name + '/' + obj_name;
    }

    //! Throws an Exception with the given message. Adds the current file path
    //! and line number to the message.
    void Throw_(const std::string &msg);
};

}  // namespace Parser
