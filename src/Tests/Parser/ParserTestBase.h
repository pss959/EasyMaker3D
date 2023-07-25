#include <string>

#include "Math/Types.h"
#include "Parser/Exception.h"
#include "Parser/Field.h"
#include "Parser/Object.h"
#include "Parser/Parser.h"
#include "Tests/SceneTestBase.h"
#include "Base/Memory.h"

// Tests that a Parser::Exception is thrown and that its message contains
// the given string pattern.
#define TEST_THROW_(STMT, PATTERN) \
    TEST_THROW(STMT, Parser::Exception, PATTERN)

// ----------------------------------------------------------------------------
// Classes for testing parsing.
// ----------------------------------------------------------------------------

enum class SimpleEnum { kE1, kE2, kE3 };
enum class FlagEnum   { kF1 = 0x1, kF2 = 0x2, kF3 = 0x4 };

// Class with all non-object parser value types.
class Simple : public Parser::Object {
  public:
    virtual void AddFields() override;

    Parser::TField<bool>          bool_val;
    Parser::TField<int>            int_val;
    Parser::TField<unsigned int>  uint_val;
    Parser::TField<float>        float_val;
    Parser::TField<std::string>    str_val;
    Parser::EnumField<SimpleEnum> enum_val;
    Parser::FlagField<FlagEnum>   flag_val;
    Parser::TField<Vector3f>     vec3f_val;
    Parser::TField<Color>        color_val;
    Parser::TField<Anglef>       angle_val;
    Parser::TField<Rotationf>      rot_val;
    Parser::VField<int>           ints_val;
    Parser::VField<std::string>   strs_val;
  protected:
    virtual ~Simple();
    friend class Parser::Registry;
};

// Derived class that adds Object and ObjectList value types.
class Derived : public Simple {
  public:
    virtual void AddFields() override;

    Parser::ObjectField<Simple>     simple;
    Parser::ObjectListField<Simple> simple_list;
    Parser::TField<int>             hidden_int;
  protected:
    Derived();
    friend class Parser::Registry;
};

// Another class for testing type errors.
class Other : public Parser::Object {
  protected:
    Other() {}
    friend class Parser::Registry;
};

// Shorthand.
DECL_SHARED_PTR(Simple);
DECL_SHARED_PTR(Derived);
DECL_SHARED_PTR(Other);

// ----------------------------------------------------------------------------
// Base class that sets up a Parser.
// ----------------------------------------------------------------------------

class ParserTestBase : public SceneTestBase {
 protected:
    Parser::Parser parser;

    // Sets up the Parser to use the Simple class.
    void InitSimple();

    // Sets up the Parser to use the Simple and Derived classes.
    void InitDerived();

    // Returns a string used to initialize all fields in a Simple instance.
    static std::string GetFullSimpleInput();

    // Parses the given string, checking for exceptions. Returns a null
    // ObjectPtr on failure.
    Parser::ObjectPtr ParseString(const std::string &input);

    // Sets up a temporary file containing the given input string, parses it,
    // and returns the result. Returns a null ObjectPtr on failure.
    Parser::ObjectPtr ParseFile(const std::string &input);

    // Tries parsing a value of a given type in a string and comparing with an
    // expected field value.
    template <typename T>
    bool TryValue(Parser::TField<T> Simple::* field, T expected,
                  const std::string &str){
        Parser::ObjectPtr obj = ParseString(str);
        if (! obj)
            return false;
        std::shared_ptr<Simple> sp = std::dynamic_pointer_cast<Simple>(obj);
        EXPECT_NOT_NULL(sp.get());
        return ((*sp).*field).GetValue() == expected;
    };
};
