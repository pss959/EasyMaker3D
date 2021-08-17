#pragma once

#include <functional>
#include <istream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "NParser/FieldSpec.h"
#include "NParser/Object.h"
#include "NParser/Value.h"
#include "NParser/ValueType.h"
#include "Util/FilePath.h"

namespace NParser {

class Scanner;

//! Parses a file or stream to produce XXXX Any errors result in a
//! Parser::Exception being thrown.
//!
//! \ingroup NParser
class Parser {
  public:
    //! Convenience typedef for a function used to create an object.
    typedef std::function<Object *()> CreationFunc;

    Parser();
    ~Parser();

    // XXXX
    void RegisterObjectType(const std::string &type_name,
                            const std::vector<FieldSpec> &field_specs,
                            const CreationFunc &creation_func);

    //! Parses the contents of the file with the given path, returning the root
    //! Object in the parse graph.
    ObjectPtr ParseFile(const Util::FilePath &path);

    //! Parses the contents of the given string, returning the root Object in
    //! the parse graph.
    ObjectPtr ParseString(const std::string &str);

  private:
    struct ObjectSpec_ {
        std::vector<FieldSpec> field_specs;
        CreationFunc           creation_func;
    };

    //! Stores an association between an Object's type name and an ObjectSpec_
    //! instance.
    std::unordered_map<std::string, ObjectSpec_> object_spec_map_;

    //! Scanner used to parse tokens.
    std::unique_ptr<Scanner> scanner_;

    //! Stack of current objects being parsed. This is implemented as a regular
    //! vector because all objects need to be accessible for constant searches.
    std::vector<ObjectPtr> object_stack_;

    //! Parses the next Object in the input.
    ObjectPtr ParseObject_();

    //! XXXX
    const ObjectSpec_ & GetObjectSpec_(const std::string &type_name);

    //! XXXX
    void ParseFields_(Object &obj, const std::vector<FieldSpec> &specs);

    // XXXX
    const FieldSpec * FindFieldSpec_(const std::vector<FieldSpec> &specs,
                                     const std::string &field_name);

    // XXXX
    void ParseAndStoreValues_(Object &obj, const FieldSpec &spec);

    // XXXX
    Value ParseValue_(ValueType type);
};

}  // namespace NParser
