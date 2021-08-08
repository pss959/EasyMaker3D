#pragma once

#include <assert.h>

#include <istream>
#include <memory>
#include <string>
#include <unordered_map>
#include <variant>

#include <ion/math/vector.h>

#include "ExceptionBase.h"
#include "Util.h"

//! XXXX Parses an XXXX file to produce a tree of stuff.
class Parser {
  public:
    //! Types of values that can be stored in a parsed Field.
    enum class ValueType {
        kBool,
        kInteger,
        kFloat,
        kString,
        kObject,        //!< Cannot be used in an ArrayField.
        kObjectList,    //!< Cannot be used in an ArrayField.
    };

    // Forward references.
    struct Field;
    struct Object;

    // Convenience typedefs.
    typedef std::shared_ptr<Field>  FieldPtr;   //!< Convenience typedef.
    typedef std::shared_ptr<Object> ObjectPtr;  //!< Convenience typedef.
    typedef std::variant<bool, int, float, std::string, ObjectPtr,
                         std::vector<ObjectPtr>> Value;

    //! Exception thrown when any loading function fails.
    class Exception : public ExceptionBase {
      public:
        Exception(const std::string &msg) : ExceptionBase(msg) {}
        Exception(const std::string &path, const std::string &msg) :
            ExceptionBase(path, "Parse error: " + msg) {}
        Exception(const std::string &path, int line_number,
                  const std::string &msg) :
            ExceptionBase(path, line_number, "Parse error: " + msg) {}
    };

    class Exception;

    //! Specification for a field to parse.
    struct FieldSpec {
        std::string name;    //!< Name of the field.
        ValueType   type;    //!< Base type of the field's value.
        uint32_t    count;   //!< Fixed number of values to parse.

        bool operator==(const FieldSpec &other) const {
            return name == other.name && type == other.type &&
                count == other.count;
        }
    };

    //! Base struct for a Field.
    struct Field {
        virtual ~Field() {}

        const FieldSpec &spec;   //!< FieldSpec for the field.

        //! The constructor is passed the FieldSpec for the field.
        Field(const FieldSpec &spec_in) : spec(spec_in) {}

        //! Returns a value of the templated type. Throws an Exception if the
        //! field type is not consistent with that type or if this is actually
        //! an ArrayField.
        template <typename T> T GetValue() const {
            if (spec.count > 1) {
                throw Exception(
                    std::string("Attempt to GetValue() for field '") +
                    spec.name + "' with count " +
                    Util::ToString(spec.count));
            }
            return std::get<T>(((SingleField *) this)->value);
        }

        //! Returns a vector of N values of the templated type. Throws an
        //! Exception if the field type is not consistent with that type or if
        //! the count in the field is not exactly N.
        template <typename T, int N> std::vector<T> GetValues() const {
            if (spec.count != N) {
                throw Exception(
                    std::string("Attempt to GetValues() with N=") +
                    Util::ToString(N) + " for field '" +
                    spec.name + "' with count " +
                    Util::ToString(spec.count));
            }
            std::vector<T> values;
            values.reserve(N);
            for (uint32_t i = 0; i < N; ++i)
                values.push_back(std::get<T>(((ArrayField *) this)->values[i]));
            return values;
        }
    };

    //! Derived field holding a single value.
    struct SingleField : public Field {
        Value value;
        SingleField(const FieldSpec &spec_in, const Value &value_in) :
            Field(spec_in), value(value_in) {
            assert(spec_in.count == 1);
        }
    };

    //! Derived field holding multiple values.
    struct ArrayField : public Field {
        std::vector<Value> values;
        ArrayField(const FieldSpec &spec_in,
                   const std::vector<Value> &values_in) :
            Field(spec_in), values(values_in) {
            assert(spec_in.count > 1);
        }
    };

    //! A parsed object.
    struct Object {
        std::string           type_name;    //!< Name of the object's type.
        std::vector<FieldPtr> fields;       //!< Fields parsed from the file.
        std::string           path;         //!< Path read from.
        int                   line_number;  //!< Line number of definition.
    };

    Parser(const std::vector<FieldSpec> &field_specs);
    ~Parser();

    ObjectPtr ParseFile(const std::string &path);
    ObjectPtr ParseStream(std::istream &in);

  private:
    //! Type for map from field name to FieldSpec.
    typedef std::unordered_map<std::string, const FieldSpec *> FieldSpecMap_;

    //! Field specs passed to the constructor.
    const std::vector<FieldSpec> &field_specs_;

    std::string   path_;            //!< Stores the path for errors.
    int           cur_line_;        //!< Current line number.
    FieldSpecMap_ field_spec_map_;

    void        BuildFieldSpecMap_();
    ObjectPtr   ParseObject_(std::istream &in);
    std::vector<ObjectPtr> ParseObjectList_(std::istream &in);
    void        ParseFields_(std::istream &in, std::vector<FieldPtr> &fields);
    FieldPtr    ParseSingleFieldValue_(std::istream &in, const FieldSpec &spec);
    FieldPtr    ParseArrayFieldValue_(std::istream &in, const FieldSpec &spec);
    Value       ParseValue_(std::istream &in, const FieldSpec &spec);
    std::string ParseName_(std::istream &in);
    bool        ParseBool_(std::istream &in);
    int         ParseInteger_(std::istream &in);
    float       ParseFloat_(std::istream &in);
    std::string ParseQuotedString_(std::istream &in);
    void        ParseChar_(std::istream &in, char expected_c);
    char        PeekChar_(std::istream &in);
    void        SkipWhiteSpace_(std::istream &in);

    const FieldSpec & GetFieldSpec_(const std::string &name);

    //! XXXX
    void Throw_(const std::string &msg);
};
