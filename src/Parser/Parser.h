#pragma once

#include <functional>
#include <istream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "Parser/Object.h"
#include "Util/FilePath.h"

namespace Parser {

class Scanner;

/// Parses a file or stream to produce instances of classes derived from
/// Parser::Object. Any failure results in a Parser::Exception being thrown.
class Parser {
  public:
    /// This struct represents a dependency created by an included file.
    struct Dependency {
        Util::FilePath including_path;
        Util::FilePath included_path;
    };

    Parser();
    ~Parser();

    /// Parses the contents of the file with the given path, returning the root
    /// Object in the parse graph.
    ObjectPtr ParseFile(const Util::FilePath &path);

    /// Parses the contents of the given string, returning the root Object in
    /// the parse graph.
    ObjectPtr ParseFromString(const std::string &str);

    /// Returns a vector of all path dependencies created by included files
    /// found during parsing.
    const std::vector<Dependency> GetDependencies() const {
        return dependencies_;
    }

  private:
    /// Convenience typedef for a map storing constants (name -> value).
    typedef std::unordered_map<std::string, std::string> ConstantsMap_;

    /// Convenience typedef for a map storing objects keyed by some name.
    typedef std::unordered_map<std::string, ObjectPtr> ObjectMap_;

    /// This struct is stored in the object_stack_. It maintains a pointer to
    /// the Object, the constants associated with it, and objects local to its
    /// scope. The constants are stored as a map from constant name to the
    /// value string. The scoped objects are accessed by name key.
    struct ObjectData_ {
        ObjectPtr     object;
        ConstantsMap_ constants_map;
        ObjectMap_    scoped_objects_map;
    };

    /// Stores Objects based on their name keys. This is the global map that
    /// stores the first instance of each name.
    ObjectMap_ object_name_map_;

    /// Stores template Objects based on their type names.
    ObjectMap_ template_map_;

    /// Scanner used to parse tokens.
    std::unique_ptr<Scanner> scanner_;

    /// Stack of ObjectData_ instances representing current objects being
    /// parsed. This is implemented as a regular vector because all objects
    /// need to be accessible for constant searches.
    std::vector<ObjectData_> object_stack_;

    /// Vector of Dependency instances created when a file is included.
    std::vector<Dependency> dependencies_;

    /// Implements most of ParseFile().
    ObjectPtr ParseFromFile_(const Util::FilePath &path);

    /// Parses the next Object in the input.
    ObjectPtr ParseObject_();

    /// Parses and returns a template object.
    ObjectPtr AddTemplate_();

    /// Parses and returns an instance of a template object.
    ObjectPtr AddInstance_();

    /// Parses and returns an object that is neither a template nor template
    /// clone. The type name is supplied. The is_template flag indicates
    /// whether the object is a template, which may affect how it is
    /// validated. If base_obj is not null, the new object should be cloned
    /// from it instead of being created from scratch. (This is used in the
    /// case of template clones).
    ObjectPtr ParseRegularObject_(const std::string &type_name,
                                  bool is_template, ObjectPtr base_obj);

    /// Parses a collection of Object instances (in square brackets, separated
    /// by commas) from the input, returning a pointer to an ObjectList.
    ObjectListPtr ParseObjectList_();

    /// Parses the contents of an included file, returning its root Object.
    ObjectPtr ParseIncludedFile_();

    /// Parses a constant definition block, storing name -> value pairs in the
    /// given ConstantsMap_.
    void ParseConstants_(Object &obj, ConstantsMap_ &map);

    /// Returns the stored ObjectPtr with the given type and name. Throws an
    /// Exception if none is found.
    const ObjectPtr & FindObject_(const std::string &type_name,
                                  const std::string &obj_name);

    /// Parses the fields of the given Object, storing values in the instance.
    void ParseFields_(Object &obj);

    /// Function used by Scanner to get the value string to substitute for a
    /// constant with the given name.
    std::string SubstituteConstant_(const std::string &name) const;

    /// Throws an exception using the Scanner.
    void Throw_(const std::string &msg) const;

    /// Builds a name key for an object from its type name and name.
    static std::string BuildObjectNameKey_(const std::string &obj_type_name,
                                           const std::string &obj_name) {
        return obj_type_name + '/' + obj_name;
    }

    /// Dumps the Object stack to standard out for help in debugging.
    void DumpObjectStack_() const;
};

}  // namespace Parser
