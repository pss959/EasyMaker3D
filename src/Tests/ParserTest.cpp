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
#include "Util/General.h"

// Tests that a Parser::Exception is thrown and that its message contains
// the given string pattern.
#define TEST_THROW_(STMT, PATTERN) \
    TEST_THROW(STMT, Parser::Exception, PATTERN)

// ----------------------------------------------------------------------------
// Classes for testing parsing.
// ----------------------------------------------------------------------------

enum class SimpleEnum { kE1, kE2, kE3 };

// Class with all simple parser value types.
class Simple : public Parser::Object {
  public:
    bool         bval;
    int          ival;
    unsigned int uival;
    float        fval;
    std::string  sval;
    SimpleEnum     eval;
    float        f3val[3];  // Example of multiple values.

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
    builder.AddBool("bval",           &Simple::bval);
    builder.AddInt("ival",            &Simple::ival);
    builder.AddUInt("uival",          &Simple::uival);
    builder.AddFloat("fval",          &Simple::fval);
    builder.AddString("sval",         &Simple::sval);
    builder.AddEnum<SimpleEnum>("eval", &Simple::eval);
    builder.AddArray<float, 3>("f3val", Parser::ValueType::kFloat,
                               &Simple::f3val);
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
        "  bval:  true,\n"
        "  ival:  -13, # In-line comment\n"
        "  uival: 67,\n"
        "  fval:  3.4,\n"
        "  sval:  \"A quoted string\",\n"
        "  eval:  \"kE2\",\n"
        "  f3val: 2 3 4.5,\n"
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
        EXPECT_TRUE(sp->bval);
        EXPECT_EQ(-13,  sp->ival);
        EXPECT_EQ(67U,  sp->uival);
        EXPECT_EQ(3.4f, sp->fval);
        EXPECT_EQ("A quoted string", sp->sval);
        EXPECT_EQ(SimpleEnum::kE2, sp->eval);
        EXPECT_EQ(2.f,  sp->f3val[0]);
        EXPECT_EQ(3.f,  sp->f3val[1]);
        EXPECT_EQ(4.5f, sp->f3val[2]);
    }
}

TEST_F(ParserTest, Derived) {
    const std::string input =
        "# Full-line comment\n"
        "Derived \"TestObj\" {\n"
        "  ival: 13, # In-line comment\n"
        "  fval: 3.4,\n"
        "  sval: \"A quoted string\",\n"
        "  f3val: 2 3 4.5,\n"
        "  simple: Simple \"Nested\" {\n"
        "     ival: 271,\n"
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
    EXPECT_EQ(13,   dp->ival);
    EXPECT_EQ(3.4f, dp->fval);
    EXPECT_EQ("A quoted string", dp->sval);
    EXPECT_EQ(2.f,  dp->f3val[0]);
    EXPECT_EQ(3.f,  dp->f3val[1]);
    EXPECT_EQ(4.5f, dp->f3val[2]);

    EXPECT_NOT_NULL(dp->simple.get());
    EXPECT_EQ("Nested", dp->simple->GetName());
    EXPECT_EQ(271, dp->simple->ival);
}

TEST_F(ParserTest, OverwriteField) {
    const std::string input =
        "Simple {\n"
        "  ival: 13,\n"
        "  ival: 19,\n"
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
    EXPECT_EQ(19, sp->ival);
}

TEST_F(ParserTest, BoolParsing) {
    InitParser<Simple>();

    auto try_func = [&](bool expected, const std::string &str){
        return TryValue(&Simple::bval, expected, str); };

    EXPECT_TRUE(try_func(false, "Simple { bval: F }"));
    EXPECT_TRUE(try_func(false, "Simple { bval: f }"));
    EXPECT_TRUE(try_func(false, "Simple { bval: False }"));
    EXPECT_TRUE(try_func(false, "Simple { bval: fAlSe }"));

    EXPECT_TRUE(try_func(true,  "Simple { bval: T }"));
    EXPECT_TRUE(try_func(true,  "Simple { bval: t }"));
    EXPECT_TRUE(try_func(true,  "Simple { bval: TruE }"));
    EXPECT_TRUE(try_func(true,  "Simple { bval: True }"));
}

TEST_F(ParserTest, IntParsing) {
    InitParser<Simple>();

    auto try_func = [&](int expected, const std::string &str){
        return TryValue(&Simple::ival, expected, str); };

    EXPECT_TRUE(try_func(10,   "Simple { ival: 10 }"));
    EXPECT_TRUE(try_func(-20,  "Simple { ival: -20 }"));
    EXPECT_TRUE(try_func(30,   "Simple { ival: +30 }"));
}

TEST_F(ParserTest, UIntParsing) {
    InitParser<Simple>();

    auto try_func = [&](unsigned int expected, const std::string &str){
        return TryValue(&Simple::uival, expected, str); };

    EXPECT_TRUE(try_func(10,   "Simple { uival: 10 }"));
    EXPECT_TRUE(try_func(62,   "Simple { uival: 076 }"));
    EXPECT_TRUE(try_func(2651, "Simple { uival: 0xa5b }"));
    EXPECT_TRUE(try_func(2651, "Simple { uival: 0XA5B }"));
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
        "  ],\n"
        "  ival:  $FOO,\n"
        "  f3val: $BAR,\n"
        "}\n";
    InitParser<Simple>();
    Parser::ObjectPtr obj = ParseString(input);
    EXPECT_NOT_NULL(obj);
    EXPECT_EQ("Simple",  obj->GetTypeName());
    std::shared_ptr<Simple> sp =
        Util::CastToDerived<Parser::Object, Simple>(obj);
    EXPECT_NOT_NULL(sp.get());
    EXPECT_EQ(123,   sp->ival);
    EXPECT_EQ(2.5f,  sp->f3val[0]);
    EXPECT_EQ(123.f, sp->f3val[1]);
    EXPECT_EQ(5.f,   sp->f3val[2]);
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
    TEST_THROW_(parser.ParseString("Simple { ival: 9 x }"),
                "Expected ',' or '}'");
    TEST_THROW_(parser.ParseString("Simple { ival: b }"),
                "Invalid integer value");
    TEST_THROW_(parser.ParseString("Simple { ival: 123b }"),
                "Invalid integer value");
    TEST_THROW_(parser.ParseString("Simple { ival: 0xa1 }"),
                "Invalid integer value");
    TEST_THROW_(parser.ParseString("Simple { uival: -12 }"),
                "Invalid unsigned integer value");
    TEST_THROW_(parser.ParseString("Simple { uival: +4 }"),
                "Invalid unsigned integer value");
    TEST_THROW_(parser.ParseString("Simple { uival: 0xqb }"),
                "Invalid unsigned integer value");
    TEST_THROW_(parser.ParseString("Simple { f3val: 12 abc 4 }"),
                "Invalid float value");
    TEST_THROW_(parser.ParseString("Simple { bval: \"glorp\" }"),
                "Invalid bool value");
    TEST_THROW_(parser.ParseString("Simple"),
                "EOF");
    TEST_THROW_(parser.ParseString("Simple { bad_field: 13 }"),
                "Unknown field");
    TEST_THROW_(parser.ParseString("<\"include/with/eof\""),
                "Expected '>', got EOF");
    TEST_THROW_(parser.ParseString("<\"\">"),
                "Invalid empty path");
}
