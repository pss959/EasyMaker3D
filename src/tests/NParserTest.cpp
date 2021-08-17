#include <filesystem>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "NParser/Exception.h"
#include "NParser/Object.h"
#include "NParser/SpecBuilder.h"
#include "NParser/Parser.h"
#include "Testing.h"
#include "Util/General.h"

// Tests that a Parser::Exception is thrown and that its message contains
// the given string pattern.
#define TEST_THROW_(STMT, PATTERN) \
    TEST_THROW(STMT, NParser::Exception, PATTERN)

// Classes for testing parsing.
class Simple : public NParser::Object {
  public:
    int         ival;
    float       fval;
    std::string sval;
    float       f3val[3];

    static std::vector<NParser::FieldSpec> GetFieldSpecs();
};

std::vector<NParser::FieldSpec> Simple::GetFieldSpecs() {
    NParser::SpecBuilder<Simple> builder;
    builder.AddSingle("ival", NParser::ValueType::kInteger,  &Simple::ival);
    builder.AddSingle("fval", NParser::ValueType::kFloat,    &Simple::fval);
    builder.AddSingle("sval", NParser::ValueType::kString,   &Simple::sval);
    builder.AddArray<float, 3>("f3val", NParser::ValueType::kFloat,
                               &Simple::f3val);
    return builder.GetSpecs();
}

class Derived : public Simple {
  public:
    std::shared_ptr<Simple>              simple;
    std::vector<std::shared_ptr<Simple>> simple_list;

    static std::vector<NParser::FieldSpec> GetFieldSpecs();
};

std::vector<NParser::FieldSpec> Derived::GetFieldSpecs() {
    NParser::SpecBuilder<Derived> builder(Simple::GetFieldSpecs());
    builder.AddObject<Simple>("simple", &Derived::simple);
    builder.AddObjectList<Simple>("simple", &Derived::simple_list);
    return builder.GetSpecs();
}

class NParserTest : public TestBase {
 protected:
    // Sets up a parser.
    NParser::Parser parser;
};

TEST_F(NParserTest, Simple) {
    const std::string input =
        "# Full-line comment\n"
        "Simple \"TestObj\" {\n"
        "  ival: 13, # In-line comment\n"
        "  fval: 3.4,\n"
        "  sval: \"A quoted string\",\n"
        "  f3val: 2 3 4.5,\n"
        "}\n";

    parser.RegisterObjectType("Simple", Simple::GetFieldSpecs(),
                              []{ return new Simple; });

    NParser::ObjectPtr obj = parser.ParseString(input);
    EXPECT_NOT_NULL(obj.get());
    EXPECT_EQ("Simple",  obj->GetTypeName());
    EXPECT_EQ("TestObj", obj->GetName());
    std::shared_ptr<Simple> sp =
        Util::CastToDerived<NParser::Object, Simple>(obj);
    EXPECT_NOT_NULL(sp.get());
    EXPECT_EQ(13,   sp->ival);
    EXPECT_EQ(3.4f, sp->fval);
    EXPECT_EQ("A quoted string", sp->sval);
    EXPECT_EQ(2.f,  sp->f3val[0]);
    EXPECT_EQ(3.f,  sp->f3val[1]);
    EXPECT_EQ(4.5f, sp->f3val[2]);
}

TEST_F(NParserTest, Derived) {
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

    parser.RegisterObjectType("Simple", Simple::GetFieldSpecs(),
                              []{ return new Simple; });
    parser.RegisterObjectType("Derived", Derived::GetFieldSpecs(),
                              []{ return new Derived; });

    NParser::ObjectPtr obj = parser.ParseString(input);
    EXPECT_NOT_NULL(obj.get());
    EXPECT_EQ("Derived", obj->GetTypeName());
    EXPECT_EQ("TestObj", obj->GetName());
    std::shared_ptr<Derived> dp =
        Util::CastToDerived<NParser::Object, Derived>(obj);
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
