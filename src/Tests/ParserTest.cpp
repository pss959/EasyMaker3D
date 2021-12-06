#include <filesystem>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "Math/Types.h"
#include "Parser/Exception.h"
#include "Parser/Field.h"
#include "Parser/Object.h"
#include "Parser/Parser.h"
#include "Parser/Registry.h"
#include "Tests/SceneTestBase.h"
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

// Class with all non-object parser value types.
class Simple : public Parser::Object {
  public:
    virtual void AddFields() override {
        AddField(bool_val);
        AddField(int_val);
        AddField(uint_val);
        AddField(float_val);
        AddField(str_val);
        AddField(enum_val);
        AddField(flag_val);
        AddField(vec3f_val);
        AddField(color_val);
        AddField(angle_val);
        AddField(rot_val);
        AddField(ints_val);
    }

    Parser::TField<bool>                   bool_val{"bool_val"};
    Parser::TField<int>                     int_val{"int_val"};
    Parser::TField<unsigned int>           uint_val{"uint_val"};
    Parser::TField<float>                 float_val{"float_val"};
    Parser::TField<std::string>             str_val{"str_val"};
    Parser::EnumField<SimpleEnum>          enum_val{"enum_val"};
    Parser::FlagField<FlagEnum>            flag_val{"flag_val"};
    Parser::TField<Vector3f>              vec3f_val{"vec3f_val"};
    Parser::TField<Color>                 color_val{"color_val"};
    Parser::TField<Anglef>                angle_val{"angle_val"};
    Parser::TField<Rotationf>               rot_val{"rot_val"};
    Parser::VField<int>                    ints_val{"ints_val"};
  protected:
    Simple() {}
    friend class Parser::Registry;
};

// Derived class that adds Object and ObjectList value types.
class Derived : public Simple {
  public:
    virtual void AddFields() override {
        Simple::AddFields();
        AddField(simple);
        AddField(simple_list);
    }

    Parser::ObjectField<Simple>     simple{"simple"};
    Parser::ObjectListField<Simple> simple_list{"simple_list"};
  protected:
    Derived() {}
    friend class Parser::Registry;
};

// Another class for testing type errors.
class Other : public Parser::Object {
  protected:
    Other() {}
    friend class Parser::Registry;
};

// ----------------------------------------------------------------------------
// Base class that sets up a Parser.
// ----------------------------------------------------------------------------

class ParserTest : public SceneTestBase {
 protected:
    Parser::Parser parser;

    // Sets up the Parser to use the Simple class.
    void InitSimple() {
        Parser::Registry::AddType<Simple>("Simple");
    }
    // Sets up the Parser to use the Simple and Derived classes.
    void InitDerived() {
        InitSimple();
        Parser::Registry::AddType<Derived>("Derived");
    }

    // Parses the given string, checking for exceptions. Returns a null
    // ObjectPtr on failure.
    Parser::ObjectPtr ParseString(const std::string &input) {
        Parser::ObjectPtr obj;
        try {
            obj = parser.ParseFromString(input);
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
            obj = parser.ParseFile(tmp_file.GetPath());
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
    bool TryValue(Parser::TField<T> Simple::* field, T expected,
                  const std::string &str){
        Parser::ObjectPtr obj = ParseString(str);
        if (! obj)
            return false;
        std::shared_ptr<Simple> sp = Util::CastToDerived<Simple>(obj);
        EXPECT_NOT_NULL(sp.get());
        return ((*sp).*field).GetValue() == expected;
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
        "  color_val: .2 .3 .4 1,\n"
        "  angle_val: 90,\n"
        "  rot_val:   0 1 0 180,\n"
        "  ints_val:  [6, 5, -2],\n"
        "}\n";

    // Set up a temporary file with the input string.
    TempFile tmp_file(input);

    // Parse both the string and the file and test the results.
    InitSimple();
    Parser::ObjectPtr obj1 = ParseString(input);
    Parser::ObjectPtr obj2 = ParseFile(input);
    EXPECT_NOT_NULL(obj1);
    EXPECT_NOT_NULL(obj2);
    for (Parser::ObjectPtr obj: { obj1, obj2 }) {
        EXPECT_NOT_NULL(obj.get());
        EXPECT_EQ("Simple",  obj->GetTypeName());
        EXPECT_EQ("TestObj", obj->GetName());
        std::shared_ptr<Simple> sp = Util::CastToDerived<Simple>(obj);
        EXPECT_NOT_NULL(sp.get());
        EXPECT_TRUE(sp->bool_val);
        EXPECT_EQ(-13,  sp->int_val);
        EXPECT_EQ(67U,  sp->uint_val);
        EXPECT_EQ(3.4f, sp->float_val);
        EXPECT_EQ("A quoted string", sp->str_val.GetValue());
        EXPECT_EQ(SimpleEnum::kE2, sp->enum_val);
        EXPECT_TRUE(sp->flag_val.GetValue().Has(FlagEnum::kF1));
        EXPECT_FALSE(sp->flag_val.GetValue().Has(FlagEnum::kF2));
        EXPECT_TRUE(sp->flag_val.GetValue().Has(FlagEnum::kF3));
        EXPECT_EQ(Vector3f(2.f, 3.f, 4.5f), sp->vec3f_val);
        EXPECT_EQ(Color(.2f, .3f, .4f, 1.f), sp->color_val);
        EXPECT_EQ(Anglef::FromDegrees(90), sp->angle_val);
        EXPECT_EQ(Rotationf::FromAxisAndAngle(Vector3f(0, 1, 0),
                                              Anglef::FromDegrees(180)),
                  sp->rot_val);
        const std::vector<int> &ints = sp->ints_val.GetValue();
        EXPECT_EQ(3U, ints.size());
        EXPECT_EQ(6,  ints[0]);
        EXPECT_EQ(5,  ints[1]);
        EXPECT_EQ(-2, ints[2]);
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
        "  simple_list: [\n"
        "     Simple \"Nested1\" {},\n"
        "     Simple \"Nested2\" {},\n"
        "  ],\n"
        "}\n";

    InitDerived();

    Parser::ObjectPtr obj = ParseString(input);
    EXPECT_NOT_NULL(obj.get());
    EXPECT_EQ("Derived", obj->GetTypeName());
    EXPECT_EQ("TestObj", obj->GetName());
    std::shared_ptr<Derived> dp = Util::CastToDerived<Derived>(obj);
    EXPECT_NOT_NULL(dp.get());
    EXPECT_EQ(13,   dp->int_val);
    EXPECT_EQ(3.4f, dp->float_val);
    EXPECT_EQ("A quoted string", dp->str_val.GetValue());
    EXPECT_EQ(Vector3f(2.f, 3.f, 4.5f), dp->vec3f_val);

    const std::shared_ptr<Simple> simp = dp->simple;
    EXPECT_NOT_NULL(simp);
    EXPECT_EQ("Nested", simp->GetName());
    EXPECT_EQ(271, simp->int_val);

    const std::vector<std::shared_ptr<Simple>> &list = dp->simple_list;
    EXPECT_EQ(2U, list.size());
    EXPECT_EQ("Nested1", list[0]->GetName());
    EXPECT_EQ("Nested2", list[1]->GetName());
}

TEST_F(ParserTest, WasSet) {
    const std::string input =
        "Simple {\n"
        "  bool_val:  True,\n"
        "  float_val: 19,\n"
        "}\n";

    // Set up a temporary file with the input string.
    TempFile tmp_file(input);

    InitSimple();
    Parser::ObjectPtr obj = ParseString(input);
    EXPECT_NOT_NULL(obj.get());
    EXPECT_EQ("Simple",  obj->GetTypeName());
    std::shared_ptr<Simple> sp = Util::CastToDerived<Simple>(obj);
    EXPECT_NOT_NULL(sp.get());
    EXPECT_TRUE(sp->bool_val.WasSet());
    EXPECT_TRUE(sp->float_val.WasSet());
    EXPECT_FALSE(sp->int_val.WasSet());
    EXPECT_FALSE(sp->uint_val.WasSet());
}

TEST_F(ParserTest, OverwriteField) {
    const std::string input =
        "Simple {\n"
        "  int_val: 13,\n"
        "  int_val: 19,\n"
        "}\n";

    // Set up a temporary file with the input string.
    TempFile tmp_file(input);

    InitSimple();
    Parser::ObjectPtr obj = ParseString(input);
    EXPECT_NOT_NULL(obj.get());
    EXPECT_EQ("Simple",  obj->GetTypeName());
    std::shared_ptr<Simple> sp = Util::CastToDerived<Simple>(obj);
    EXPECT_NOT_NULL(sp.get());
    EXPECT_EQ(19, sp->int_val);
}

TEST_F(ParserTest, BoolParsing) {
    InitSimple();

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
    InitSimple();

    auto try_func = [&](int expected, const std::string &str){
        return TryValue(&Simple::int_val, expected, str); };

    EXPECT_TRUE(try_func(10,   "Simple { int_val: 10 }"));
    EXPECT_TRUE(try_func(-20,  "Simple { int_val: -20 }"));
    EXPECT_TRUE(try_func(30,   "Simple { int_val: +30 }"));
}

TEST_F(ParserTest, UIntParsing) {
    InitSimple();

    auto try_func = [&](unsigned int expected, const std::string &str){
        return TryValue(&Simple::uint_val, expected, str); };

    EXPECT_TRUE(try_func(10,   "Simple { uint_val: 10 }"));
    EXPECT_TRUE(try_func(62,   "Simple { uint_val: 076 }"));
    EXPECT_TRUE(try_func(2651, "Simple { uint_val: 0xa5b }"));
    EXPECT_TRUE(try_func(2651, "Simple { uint_val: 0XA5B }"));
}

TEST_F(ParserTest, StringParsing) {
    InitSimple();

    auto try_func = [&](const std::string &expected, const std::string &str){
        return TryValue(&Simple::str_val, expected, str); };

    EXPECT_TRUE(try_func("Hello",   "Simple { str_val: \"Hello\" }"));
    EXPECT_TRUE(try_func("A \"Q\"", "Simple { str_val: \"A \\\"Q\\\"\" }"));
}

TEST_F(ParserTest, ColorParsing) {
    InitSimple();

    auto try_func = [&](const Color &expected, const std::string &str){
        return TryValue(&Simple::color_val, expected, str); };

    EXPECT_TRUE(try_func(Color(1, 1, 1, 1),
                         "Simple { color_val: \"#ffffffff\" }"));
    EXPECT_TRUE(try_func(Color(1, 1, 1, 1),
                         "Simple { color_val: \"#ffffff\" }"));
    EXPECT_TRUE(try_func(Color(1, 1, 1, 1),
                         "Simple { color_val: 1 1 1 1 }"));
    EXPECT_TRUE(try_func(Color(0, 1, .5f, 1),
                         "Simple { color_val: 0 1 .5 1 }"));
    EXPECT_TRUE(try_func(Color(1, 1, 1, 1),
                         "Simple { color_val: 255 255 255 255 }"));
    EXPECT_TRUE(try_func(Color(1, 0, 0, 1),
                         "Simple { color_val: 255 0 0 255 }"));
}

TEST_F(ParserTest, Includes) {
    TempFile file1("Simple \"Child1\" {}");
    TempFile file2("Simple \"Child2\" {}");
    const std::string input =
        "Derived \"ParentName\" { \n"
        "  simple_list: [\n"
        "      <\"" + file1.GetPath().ToString() + "\">,\n"
        "      <\"" + file2.GetPath().ToString() + "\">,\n"
        "  ],\n"
        "}\n";

    InitDerived();
    Parser::ObjectPtr obj = ParseString(input);
    EXPECT_NOT_NULL(obj.get());

    EXPECT_EQ("Derived", obj->GetTypeName());
    EXPECT_EQ("ParentName", obj->GetName());
    std::shared_ptr<Derived> dp = Util::CastToDerived<Derived>(obj);
    EXPECT_NOT_NULL(dp.get());
    const std::vector<std::shared_ptr<Simple>> &list = dp->simple_list;
    EXPECT_FALSE(list.empty());
    EXPECT_EQ(2U, list.size());
    EXPECT_EQ("Child1", list[0]->GetName());
    EXPECT_EQ("Child2", list[1]->GetName());
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
    InitSimple();
    Parser::ObjectPtr obj = ParseString(input);
    EXPECT_NOT_NULL(obj);
    EXPECT_EQ("Simple",  obj->GetTypeName());
    std::shared_ptr<Simple> sp = Util::CastToDerived<Simple>(obj);
    EXPECT_NOT_NULL(sp.get());
    EXPECT_EQ(123, sp->int_val);
    EXPECT_EQ(Vector3f(2.5f, 123.f, 5.f), sp->vec3f_val);
    EXPECT_EQ(456.f, sp->float_val);
}

TEST_F(ParserTest, Templates) {
    const std::string input =
        "Derived {\n"
        "  simple: TEMPLATE Simple \"TempName\" {\n"
        "    int_val:   32,\n"
        "    float_val: 12.5,\n"
        "  },\n"
        "  simple_list: [\n"
        // This instance should use both template values.
        "    INSTANCE \"TempName\" \"Inst1\" {},\n"
        // This instance should override int_val.
        "    INSTANCE \"TempName\" \"Inst2\" {\n"
        "      int_val: 271,\n"
        "    },\n"
        "  ],\n"
        "}\n";

    InitDerived();
    Parser::ObjectPtr obj = ParseString(input);
    EXPECT_NOT_NULL(obj.get());
    std::shared_ptr<Derived> dp = Util::CastToDerived<Derived>(obj);
    EXPECT_NOT_NULL(dp.get());

    // Validate the template is untouched.
    const std::shared_ptr<Simple> temp = dp->simple;
    EXPECT_NOT_NULL(temp);
    EXPECT_EQ("TempName", temp->GetName());
    EXPECT_EQ(32,   temp->int_val);
    EXPECT_EQ(12.5, temp->float_val);
    EXPECT_TRUE(temp->IsTemplate());

    // Validate the instances of the template.
    const std::vector<std::shared_ptr<Simple>> &list = dp->simple_list;
    EXPECT_EQ(2U, list.size());
    const std::shared_ptr<Simple> inst1 = list[0];
    const std::shared_ptr<Simple> inst2 = list[1];
    EXPECT_EQ("Inst1", inst1->GetName());
    EXPECT_EQ(32,   inst1->int_val);
    EXPECT_EQ(12.5, inst1->float_val);
    EXPECT_EQ("Inst2", inst2->GetName());
    EXPECT_EQ(271,  inst2->int_val);    // Override template value.
    EXPECT_EQ(12.5, inst2->float_val);  // Inherit template value.
    EXPECT_EQ(Parser::Object::ObjType::kInstance, inst1->GetObjectType());
    EXPECT_EQ(Parser::Object::ObjType::kInstance, inst2->GetObjectType());
}

TEST_F(ParserTest, Scoping) {
    const std::string input =
        "Derived \"D1\" {\n"                  // dp1
        "  simple: Simple \"S1\" {},\n"       // sp1
        "  simple_list: [\n"
        "    USE Simple \"S1\",\n"            // Should be instance of sp1
        "    Derived \"D2\" {\n"              // dp2
        "      simple_list: [\n"
        "        Simple \"S1\" {},\n"         // sp2
        "        USE Simple \"S1\",\n"        // Should be instance of sp2
        "      ],\n"
        "    },\n"
        "    Derived \"D3\" {\n"              // dp3
        "      simple: USE Simple \"S1\",\n"  // Should be instance of sp1
        "    },\n"
        "  ],\n"
        "}\n";

    InitDerived();
    Parser::ObjectPtr obj = ParseString(input);
    EXPECT_NOT_NULL(obj.get());
    std::shared_ptr<Derived> dp1 = Util::CastToDerived<Derived>(obj);
    EXPECT_NOT_NULL(dp1.get());

    const auto sp1 = dp1->simple.GetValue();
    const auto &list1 = dp1->simple_list.GetValue();
    EXPECT_EQ(3U, list1.size());
    EXPECT_EQ_OBJS(sp1, list1[0]);

    const auto dp2 = Util::CastToDerived<Derived>(list1[1]);
    EXPECT_NOT_NULL(dp2.get());
    const auto &list2 = dp2->simple_list.GetValue();
    EXPECT_EQ(2U, list2.size());
    const auto sp2 = list2[0];
    EXPECT_NE(sp1, sp2);
    EXPECT_EQ_OBJS(sp2, list2[1]);

    const auto dp3 = Util::CastToDerived<Derived>(list1[2]);
    EXPECT_NOT_NULL(dp3.get());
    EXPECT_EQ_OBJS(sp1, dp3->simple.GetValue());
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
    InitSimple();
    TEST_THROW_(InitSimple(), "Object type registered more than once");
}

TEST_F(ParserTest, BadReference) {
    TempFile file1("");
    TempFile file2("Simple \"Child2\" {}");
    const std::string input =
        "Derived { \n"
        "  simple_list: [\n"
        "      Simple \"Child1\" {},\n"
        "      USE Simple \"Child2\",\n"  // Bad reference.
        "  ],\n"
        "}\n";
    InitDerived();
    TEST_THROW_(parser.ParseFromString(input), "Invalid reference to object");
}

TEST_F(ParserTest, SyntaxErrors) {
    InitDerived();
    Parser::Registry::AddType<Other>("Other");

    TEST_THROW_(parser.ParseFromString(" "),
                "Invalid empty name for object type");
    TEST_THROW_(parser.ParseFromString("Simplex"),
                "Unknown object type");
    TEST_THROW_(parser.ParseFromString("Simple ="),
                "Expected '{'");
    TEST_THROW_(parser.ParseFromString("Simple { int_val: 9 x }"),
                "Expected ',' or '}'");
    TEST_THROW_(parser.ParseFromString("Simple { int_val: b }"),
                "Invalid integer value");
    TEST_THROW_(parser.ParseFromString("Simple { int_val: 123b }"),
                "Invalid integer value");
    TEST_THROW_(parser.ParseFromString("Simple { int_val: 0xa1 }"),
                "Invalid integer value");
    TEST_THROW_(parser.ParseFromString("Simple { uint_val: -12 }"),
                "Invalid unsigned integer value");
    TEST_THROW_(parser.ParseFromString("Simple { uint_val: +4 }"),
                "Invalid unsigned integer value");
    TEST_THROW_(parser.ParseFromString("Simple { uint_val: 0xqb }"),
                "Invalid unsigned integer value");
    TEST_THROW_(parser.ParseFromString("Simple { vec3f_val: 12 abc 4 }"),
                "Invalid float value");
    TEST_THROW_(parser.ParseFromString("Simple { bool_val: \"glorp\" }"),
                "Invalid bool value");
    TEST_THROW_(parser.ParseFromString("Simple { enum_val: \"glorp\" }"),
                "Invalid value for enum");
    TEST_THROW_(parser.ParseFromString("Simple { flag_val: \"glorp\" }"),
                "Invalid value for flag enum");
    TEST_THROW_(parser.ParseFromString("Simple { flag_val: \"kF1|x\" }"),
                "Invalid value for flag enum");
    TEST_THROW_(parser.ParseFromString("Simple"),
                "EOF");
    TEST_THROW_(parser.ParseFromString("Simple { bad_field: 13 }"),
                "Unknown field");
    TEST_THROW_(parser.ParseFromString("<\"include/with/eof\""),
                "Expected '>', got EOF");
    TEST_THROW_(parser.ParseFromString("<\"\">"),
                "Invalid empty path");

    TEST_THROW_(parser.ParseFromString("Derived { simple: Other {} }"),
                "Incorrect object type");
    TEST_THROW_(parser.ParseFromString("Derived { simple_list: [Other {}] }"),
                "Incorrect object type");
}
