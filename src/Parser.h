#pragma once

#include <istream>
#include <memory>
#include <string>
#include <unordered_map>

#include <ion/math/vector.h>

#include "ExceptionBase.h"

//! XXXX Parses an XXXX file to produce a tree of stuff.
class Parser {
  public:
    struct Object;  // Forward reference.
    typedef std::shared_ptr<Object> ObjectPtr;  //!< Convenience typedef.

    //! Exception thrown when any loading function fails.
    class Exception : public ExceptionBase {
      public:
        Exception(const std::string &path, const std::string &msg) :
            ExceptionBase(path, "Parse error: " + msg) {}
        Exception(const std::string &path, int line_number,
                  const std::string &msg) :
            ExceptionBase(path, line_number, "Parse error: " + msg) {}
    };

    struct Field {
        enum class Type {
            kBool,
            kString,
            kInteger,
            kScalar,
            kVector2,
            kVector3,
            kVector4,
            kObject,
            kObjects,
        };
        std::string            name;
        Type                   type;
        bool                   bool_val;
        std::string            string_val;
        int                    integer_val;
        float                  scalar_val;
        ion::math::Vector2f    vector2_val;
        ion::math::Vector3f    vector3_val;
        ion::math::Vector4f    vector4_val;
        ObjectPtr              object_val;
        std::vector<ObjectPtr> objects_val;
    };

    struct Object {
        std::string        type_name;
        std::vector<Field> fields;
        std::string        path;         //!< Path read from.
        int                line_number;  //!< Line number of definition.
    };

    //! Type for field name to field type map.
    typedef std::unordered_map<std::string, Field::Type> FieldTypeMap;

    Parser();
    ~Parser();

    //! XXXX
    ObjectPtr ParseFile(const std::string &path,
                        const FieldTypeMap &field_type_map);

  private:
    std::string         path_;            //!< Stores the path for errors.
    int                 cur_line_;        //!< Current line number.
    const FieldTypeMap *field_type_map_;  //!< Maps field names to types.

    ObjectPtr   ParseObject_(std::istream &in);
    void        ParseObjects_(std::istream &in,
                              std::vector<ObjectPtr> &objects);
    void        ParseFields_(std::istream &in, std::vector<Field> &fields);
    void        ParseFieldValue_(std::istream &in, Field &field);
    std::string ParseName_(std::istream &in);
    bool        ParseBool_(std::istream &in);
    std::string ParseQuotedString_(std::istream &in);
    void        ParseChar_(std::istream &in, char expected_c);
    char        PeekChar_(std::istream &in);
    void        SkipWhiteSpace_(std::istream &in);

    void        InitFieldTypeMap_();
    Field::Type GetFieldType_(const std::string &name);

    //! XXXX
    void Throw_(const std::string &msg);
};
