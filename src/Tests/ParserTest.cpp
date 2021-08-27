#include <filesystem>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "Parser/Exception.h"
#include "Parser/Object.h"
#include "Parser/SpecBuilder.h"
#include "Parser/Parser.h"
#include "Testing.h"
#include "Util/Enum.h"
#include "Util/General.h"

// Tests that a Parser::Exception is thrown and that its message contains
// the given string pattern.
#define TEST_THROW_(STMT, PATTERN) \
    TEST_THROW(STMT, Parser::Exception, PATTERN)

// ----------------------------------------------------------------------------
// Classes for testing parsing.
// ----------------------------------------------------------------------------

enum class SimpleEnum { kE1, kE2, kE3 };
enum class FlagEnum   { kF1 = 0x1, kF2 = 0x2, kF3 = 0x4 };

// Class with all simple parser value types.
class Simple : public Parser::Object {
  public:
    bool                   bool_val;
    int                     int_val;
    unsigned int           uint_val;
    float                 float_val;
    std::string             str_val;
    SimpleEnum             enum_val;
    Util::Flags<FlagEnum>  flag_val;
    float                 vec3f_val[3];  // Example of multiple values.

    static Parser::ObjectSpec GetObjectSpec();
};

// Derived class that adds kObject and kObjectList value types.
class Derived : public Simple {
  public:
    std::shared_ptr<Simple>              simple;
    std::vector<std::shared_ptr<Simple>> simple_list;

    static Parser::ObjectSpec GetObjectSpec();
};

// ----------------------------------------------------------------------------
// ObjectSpec function for each class.
// ----------------------------------------------------------------------------

Parser::ObjectSpec Simple::GetObjectSpec() {
    Parser::SpecBuilder<Simple> builder;
    builder.AddBool("bool_val",             &Simple::bool_val);
    builder.AddInt("int_val",               &Simple::int_val);
    builder.AddUInt("uint_val",             &Simple::uint_val);
    builder.AddFloat("float_val",           &Simple::float_val);
    builder.AddString("str_val",            &Simple::str_val);
    builder.AddEnum<SimpleEnum>("enum_val", &Simple::enum_val);
    builder.AddFlags<FlagEnum>("flag_val",  &Simple::flag_val);
    builder.AddArray<float, 3>("vec3f_val", Parser::ValueType::kFloat,
                               &Simple::vec3f_val);
    return Parser::ObjectSpec{
        "Simple", false, []{ return new Simple; }, builder.GetSpecs() };
}

Parser::ObjectSpec Derived::GetObjectSpec() {
    Parser::SpecBuilder<Derived> builder(Simple::GetObjectSpec().field_specs);
    builder.AddObject<Simple>("simple",          &Derived::simple);
    builder.AddObjectList<Simple>("simple_list", &Derived::simple_list);
    return Parser::ObjectSpec{
        "Derived", false, []{ return new Derived; }, builder.GetSpecs() };
}

// ----------------------------------------------------------------------------
// Base class that sets up a Parser with specs for a given class.
// ----------------------------------------------------------------------------

class ParserTest : public TestBase {
 protected:
    Parser::Parser parser;
    template <typename T> void InitParser() {
        // Sets up a parser.
        parser.RegisterObjectType(T::GetObjectSpec());
    }

    // Parses the given string, checking for exceptions. Returns a null
    // ObjectPtr on failure.
    Parser::ObjectPtr ParseString(const std::string &input) {
        Parser::ObjectPtr obj;
        try {
            obj = parser.ParseString(input);
        }
        catch (const Parser::Exception &ex) {
            std::cerr << "*** EXCEPTION; " << ex.what() << "\n";
            obj.reset();
        }
        return obj;
    }

    // Sets up a temporary file containing the given input string, parses it,
    // and returns the result. Returns a null ObjectPtr on failure.
    Parser::ObjectPtr ParseFile(const std::string &input) {
        TempFile tmp_file(input);
        Parser::ObjectPtr obj;
        try {
            obj = parser.ParseFile(tmp_file.GetPathString());
        }
        catch (const Parser::Exception &ex) {
            std::cerr << "*** EXCEPTION; " << ex.what() << "\n";
            obj.reset();
        }
        return obj;
    }

    // Tries parsing a value of a given type in a string and comparing with an
    // expected field value.
    template <typename T>
    bool TryValue(T Simple::* field, T expected, const std::string &str){
        Parser::ObjectPtr obj = ParseString(str);
        if (! obj)
            return false;
        std::shared_ptr<Simple> sp =
            Util::CastToDerived<Parser::Object, Simple>(obj);
        EXPECT_NOT_NULL(sp.get());
        return ((*sp).*field) == expected;
    };
};

// ----------------------------------------------------------------------------
// The tests.
// ----------------------------------------------------------------------------

TEST_F(ParserTest, StringAndFile) {
    const std::string input =
        "# Full-line comment\n"
        "Simple \"TestObj\" {\n"
        "  bool_val:  true,\n"
        "  int_val:   -13, # In-line comment\n"
        "  uint_val:  67,\n"
        "  float_val: 3.4,\n"
        "  str_val:   \"A quoted string\",\n"
        "  enum_val:  \"kE2\",\n"
        "  flag_val:  \"kF3| kF1\",\n"
        "  vec3f_val: 2 3 4.5,\n"
        "}\n";

    // Set up a temporary file with the input string.
    TempFile tmp_file(input);

    // Parse both the string and the file and test the results.
    InitParser<Simple>();
    Parser::ObjectPtr obj1 = ParseString(input);
    Parser::ObjectPtr obj2 = ParseFile(input);
    EXPECT_NOT_NULL(obj1);
    EXPECT_NOT_NULL(obj2);
    for (Parser::ObjectPtr obj: { obj1, obj2 }) {
        EXPECT_NOT_NULL(obj.get());
        EXPECT_EQ("Simple",  obj->GetTypeName());
        EXPECT_EQ("TestObj", obj->GetName());
        std::shared_ptr<Simple> sp =
            Util::CastToDerived<Parser::Object, Simple>(obj);
        EXPECT_NOT_NULL(sp.get());
        EXPECT_TRUE(sp->bool_val);
        EXPECT_EQ(-13,  sp->int_val);
        EXPECT_EQ(67U,  sp->uint_val);
        EXPECT_EQ(3.4f, sp->float_val);
        EXPECT_EQ("A quoted string", sp->str_val);
        EXPECT_EQ(SimpleEnum::kE2, sp->enum_val);
        EXPECT_TRUE(sp->flag_val.Has(FlagEnum::kF1));
        EXPECT_FALSE(sp->flag_val.Has(FlagEnum::kF2));
        EXPECT_TRUE(sp->flag_val.Has(FlagEnum::kF3));
        EXPECT_EQ(2.f,  sp->vec3f_val[0]);
        EXPECT_EQ(3.f,  sp->vec3f_val[1]);
        EXPECT_EQ(4.5f, sp->vec3f_val[2]);
    }
}

TEST_F(ParserTest, Derived) {
    const std::string input =
        "# Full-line comment\n"
        "Derived \"TestObj\" {\n"
        "  int_val:   13, # In-line comment\n"
        "  float_val: 3.4,\n"
        "  str_val:   \"A quoted string\",\n"
        "  vec3f_val: 2 3 4.5,\n"
        "  simple: Simple \"Nested\" {\n"
        "     int_val: 271,\n"
        "  },\n"
        "}\n";

    InitParser<Simple>();
    InitParser<Derived>();

    Parser::ObjectPtr obj = ParseString(input);
    EXPECT_NOT_NULL(obj.get());
    EXPECT_EQ("Derived", obj->GetTypeName());
    EXPECT_EQ("TestObj", obj->GetName());
    std::shared_ptr<Derived> dp =
        Util::CastToDerived<Parser::Object, Derived>(obj);
    EXPECT_NOT_NULL(dp.get());
    EXPECT_EQ(13,   dp->int_val);
    EXPECT_EQ(3.4f, dp->float_val);
    EXPECT_EQ("A quoted string", dp->str_val);
    EXPECT_EQ(2.f,  dp->vec3f_val[0]);
    EXPECT_EQ(3.f,  dp->vec3f_val[1]);
    EXPECT_EQ(4.5f, dp->vec3f_val[2]);

    EXPECT_NOT_NULL(dp->simple.get());
    EXPECT_EQ("Nested", dp->simple->GetName());
    EXPECT_EQ(271, dp->simple->int_val);
}

TEST_F(ParserTest, OverwriteField) {
    const std::string input =
        "Simple {\n"
        "  int_val: 13,\n"
        "  int_val: 19,\n"
        "}\n";

    // Set up a temporary file with the input string.
    TempFile tmp_file(input);

    InitParser<Simple>();
    Parser::ObjectPtr obj = ParseString(input);
    EXPECT_NOT_NULL(obj.get());
    EXPECT_EQ("Simple",  obj->GetTypeName());
    std::shared_ptr<Simple> sp =
        Util::CastToDerived<Parser::Object, Simple>(obj);
    EXPECT_NOT_NULL(sp.get());
    EXPECT_EQ(19, sp->int_val);
}

TEST_F(ParserTest, BoolParsing) {
    InitParser<Simple>();

    auto try_func = [&](bool expected, const std::string &str){
        return TryValue(&Simple::bool_val, expected, str); };

    EXPECT_TRUE(try_func(false, "Simple { bool_val: F }"));
    EXPECT_TRUE(try_func(false, "Simple { bool_val: f }"));
    EXPECT_TRUE(try_func(false, "Simple { bool_val: False }"));
    EXPECT_TRUE(try_func(false, "Simple { bool_val: fAlSe }"));

    EXPECT_TRUE(try_func(true,  "Simple { bool_val: T }"));
    EXPECT_TRUE(try_func(true,  "Simple { bool_val: t }"));
    EXPECT_TRUE(try_func(true,  "Simple { bool_val: TruE }"));
    EXPECT_TRUE(try_func(true,  "Simple { bool_val: True }"));
}

TEST_F(ParserTest, IntParsing) {
    InitParser<Simple>();

    auto try_func = [&](int expected, const std::string &str){
        return TryValue(&Simple::int_val, expected, str); };

    EXPECT_TRUE(try_func(10,   "Simple { int_val: 10 }"));
    EXPECT_TRUE(try_func(-20,  "Simple { int_val: -20 }"));
    EXPECT_TRUE(try_func(30,   "Simple { int_val: +30 }"));
}

TEST_F(ParserTest, UIntParsing) {
    InitParser<Simple>();

    auto try_func = [&](unsigned int expected, const std::string &str){
        return TryValue(&Simple::uint_val, expected, str); };

    EXPECT_TRUE(try_func(10,   "Simple { uint_val: 10 }"));
    EXPECT_TRUE(try_func(62,   "Simple { uint_val: 076 }"));
    EXPECT_TRUE(try_func(2651, "Simple { uint_val: 0xa5b }"));
    EXPECT_TRUE(try_func(2651, "Simple { uint_val: 0XA5B }"));
}

TEST_F(ParserTest, Includes) {
    TempFile file1("Simple \"Child1\" {}");
    TempFile file2("Simple \"Child2\" {}");
    const std::string input =
        "Derived \"ParentName\" { \n"
        "  simple_list: [\n"
        "      <\"" + file1.GetPathString() + "\">,\n"
        "      <\"" + file2.GetPathString() + "\">,\n"
        "  ],\n"
        "}\n";

    InitParser<Simple>();
    InitParser<Derived>();
    Parser::ObjectPtr obj = ParseString(input);
    EXPECT_NOT_NULL(obj.get());

    EXPECT_EQ("Derived", obj->GetTypeName());
    EXPECT_EQ("ParentName", obj->GetName());
    std::shared_ptr<Derived> dp =
        Util::CastToDerived<Parser::Object, Derived>(obj);
    EXPECT_NOT_NULL(dp.get());
    EXPECT_FALSE(dp->simple_list.empty());
    EXPECT_EQ(2U, dp->simple_list.size());
    EXPECT_EQ("Child1", dp->simple_list[0]->GetName());
    EXPECT_EQ("Child2", dp->simple_list[1]->GetName());
}

TEST_F(ParserTest, Constants) {
    const std::string input =
        "Simple {\n"
        "  [\n"
        "     FOO: \"123\",\n"
        "     BAR: \"2.5 $FOO 5.0\",\n"
        "     FOO_0: \"456\",\n"
        "  ],\n"
        "  int_val:  $FOO,\n"
        "  vec3f_val: $BAR,\n"
        "  float_val: $FOO_0,\n"
        "}\n";
    InitParser<Simple>();
    Parser::ObjectPtr obj = ParseString(input);
    EXPECT_NOT_NULL(obj);
    EXPECT_EQ("Simple",  obj->GetTypeName());
    std::shared_ptr<Simple> sp =
        Util::CastToDerived<Parser::Object, Simple>(obj);
    EXPECT_NOT_NULL(sp.get());
    EXPECT_EQ(123,   sp->int_val);
    EXPECT_EQ(2.5f,  sp->vec3f_val[0]);
    EXPECT_EQ(123.f, sp->vec3f_val[1]);
    EXPECT_EQ(5.f,   sp->vec3f_val[2]);
    EXPECT_EQ(456.f, sp->float_val);
}

// ----------------------------------------------------------------------------
// Error tests. These do not use the ParserTest functions that catch
// exceptions.
// ----------------------------------------------------------------------------

TEST_F(ParserTest, BadFile) {
    TEST_THROW_(parser.ParseFile("/no/such/file/exists"),
                "Failed to open file");
}

TEST_F(ParserTest, ObjectTypeConflict) {
    InitParser<Simple>();
    TEST_THROW_(InitParser<Simple>(), "Object type registered more than once");
}

TEST_F(ParserTest, BadReference) {
    TempFile file1("");
    TempFile file2("Simple \"Child2\" {}");
    const std::string input =
        "Derived { \n"
        "  simple_list: [\n"
        "      Simple \"Child1\" {},\n"
        "      Simple \"Child2\";,\n"  // Bad reference.
        "  ],\n"
        "}\n";
    InitParser<Simple>();
    InitParser<Derived>();
    TEST_THROW_(parser.ParseString(input), "Invalid reference to object");
}

TEST_F(ParserTest, SyntaxErrors) {
    InitParser<Simple>();
    TEST_THROW_(parser.ParseString(" "),
                "Invalid empty type name");
    TEST_THROW_(parser.ParseString("Simplex"),
                "Unknown object type");
    TEST_THROW_(parser.ParseString("Simple ="),
                "Expected '{'");
    TEST_THROW_(parser.ParseString("Simple { int_val: 9 x }"),
                "Expected ',' or '}'");
    TEST_THROW_(parser.ParseString("Simple { int_val: b }"),
                "Invalid integer value");
    TEST_THROW_(parser.ParseString("Simple { int_val: 123b }"),
                "Invalid integer value");
    TEST_THROW_(parser.ParseString("Simple { int_val: 0xa1 }"),
                "Invalid integer value");
    TEST_THROW_(parser.ParseString("Simple { uint_val: -12 }"),
                "Invalid unsigned integer value");
    TEST_THROW_(parser.ParseString("Simple { uint_val: +4 }"),
                "Invalid unsigned integer value");
    TEST_THROW_(parser.ParseString("Simple { uint_val: 0xqb }"),
                "Invalid unsigned integer value");
    TEST_THROW_(parser.ParseString("Simple { vec3f_val: 12 abc 4 }"),
                "Invalid float value");
    TEST_THROW_(parser.ParseString("Simple { bool_val: \"glorp\" }"),
                "Invalid bool value");
    TEST_THROW_(parser.ParseString("Simple { enum_val: \"glorp\" }"),
                "Invalid value for enum");
    TEST_THROW_(parser.ParseString("Simple { flag_val: \"glorp\" }"),
                "Invalid value for flag enum");
    TEST_THROW_(parser.ParseString("Simple { flag_val: \"kF1|x\" }"),
                "Invalid value for flag enum");
    TEST_THROW_(parser.ParseString("Simple"),
                "EOF");
    TEST_THROW_(parser.ParseString("Simple { bad_field: 13 }"),
                "Unknown field");
    TEST_THROW_(parser.ParseString("<\"include/with/eof\""),
                "Expected '>', got EOF");
    TEST_THROW_(parser.ParseString("<\"\">"),
                "Invalid empty path");
}
