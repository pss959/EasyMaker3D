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

//! Parses a file or stream to produce instances of classes derived from
//! NParser::Object. Any failure results in a NParser::Exception being thrown.
class Parser {
  public:
    //! Convenience typedef for a function used to create an object.
    typedef std::function<Object *()> CreationFunc;

    Parser();
    ~Parser();

    //! Registers a derived Object class that can be parsed. The type_name
    //! parameter is the name that will be associated with the class in the
    //! parsed data. The FieldSpec vector specifies what fields can be parsed
    //! for an instance and how to store the parsed values. The creation_func
    //! parameter defines a function that is invoked to create an instance of
    //! the class.
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

    //! Stores Objects based on their name keys.
    std::unordered_map<std::string, ObjectPtr>   object_name_map_;

    //! Scanner used to parse tokens.
    std::unique_ptr<Scanner> scanner_;

    //! Stack of current objects being parsed. This is implemented as a regular
    //! vector because all objects need to be accessible for constant searches.
    std::vector<ObjectPtr> object_stack_;

    //! Parses the next Object in the input.
    ObjectPtr ParseObject_();

    //! Parses a collection of Object instances (in square brackets, separated
    //! by commas) from the input.
    std::vector<ObjectPtr> ParseObjectList_();

    //! Returns the stored ObjectPtr with the given type and name. Throws an
    //! Exception if none is found.
    const ObjectPtr & FindObject_(const std::string &type_name,
                                  const std::string &obj_name);

    //! Returns the ObjectSpec_ instance for the given type. Throws an
    //! Exception if none is found.
    const ObjectSpec_ & GetObjectSpec_(const std::string &type_name);

    //! Parses the fields of the given Object, storing values in the instance
    //! based on the store functions in the FieldSpec instances.
    void ParseFields_(Object &obj, const std::vector<FieldSpec> &specs);

    //! Returns the FieldSpec in the given vector that matches
    //! field_name. Throws an Exception if it is not found. The Object is
    //! supplied so that it can be used for the Exception message.
    const FieldSpec & FindFieldSpec_(const Object &obj,
                                     const std::vector<FieldSpec> &specs,
                                     const std::string &field_name);

    //! Parses the value(s) for the field with the given FieldSpec, storing the
    //! results in the Object. Throws an Exception if anything goes wrong.
    void ParseAndStoreValues_(Object &obj, const FieldSpec &spec);

    //! Parses and returns a single Value of the given type. Throws an
    //! Exception if anything goes wrong.
    Value ParseValue_(ValueType type);

    //! Builds a name key for an object from its type name and name.
    static std::string BuildObjectNameKey_(const std::string &obj_type_name,
                                           const std::string &obj_name) {
        return obj_type_name + '/' + obj_name;
    }
};

}  // namespace NParser
