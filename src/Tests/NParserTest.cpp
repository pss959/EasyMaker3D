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

// Classes for testing parsing.
class Simple : public Parser::Object {
  public:
    int         ival;
    float       fval;
    std::string sval;
    float       f3val[3];

    static Parser::ObjectSpec GetObjectSpec();
};

Parser::ObjectSpec Simple::GetObjectSpec() {
    Parser::SpecBuilder<Simple> builder;
    builder.AddSingle("ival", Parser::ValueType::kInteger,  &Simple::ival);
    builder.AddSingle("fval", Parser::ValueType::kFloat,    &Simple::fval);
    builder.AddSingle("sval", Parser::ValueType::kString,   &Simple::sval);
    builder.AddArray<float, 3>("f3val", Parser::ValueType::kFloat,
                               &Simple::f3val);
    return Parser::ObjectSpec{
        "Simple", false, []{ return new Simple; }, builder.GetSpecs() };
}

class Derived : public Simple {
  public:
    std::shared_ptr<Simple>              simple;
    std::vector<std::shared_ptr<Simple>> simple_list;

    static Parser::ObjectSpec GetObjectSpec();
};

Parser::ObjectSpec Derived::GetObjectSpec() {
    Parser::SpecBuilder<Derived> builder(Simple::GetObjectSpec().field_specs);
    builder.AddObject<Simple>("simple", &Derived::simple);
    builder.AddObjectList<Simple>("simple", &Derived::simple_list);
    return Parser::ObjectSpec{
        "Derived", false, []{ return new Derived; }, builder.GetSpecs() };
}

class ParserTest : public TestBase {
 protected:
    // Sets up a parser.
    Parser::Parser parser;
};

TEST_F(ParserTest, Simple) {
    const std::string input =
        "# Full-line comment\n"
        "Simple \"TestObj\" {\n"
        "  ival: 13, # In-line comment\n"
        "  fval: 3.4,\n"
        "  sval: \"A quoted string\",\n"
        "  f3val: 2 3 4.5,\n"
        "}\n";

    parser.RegisterObjectType(Simple::GetObjectSpec());

    Parser::ObjectPtr obj = parser.ParseString(input);
    EXPECT_NOT_NULL(obj.get());
    EXPECT_EQ("Simple",  obj->GetTypeName());
    EXPECT_EQ("TestObj", obj->GetName());
    std::shared_ptr<Simple> sp =
        Util::CastToDerived<Parser::Object, Simple>(obj);
    EXPECT_NOT_NULL(sp.get());
    EXPECT_EQ(13,   sp->ival);
    EXPECT_EQ(3.4f, sp->fval);
    EXPECT_EQ("A quoted string", sp->sval);
    EXPECT_EQ(2.f,  sp->f3val[0]);
    EXPECT_EQ(3.f,  sp->f3val[1]);
    EXPECT_EQ(4.5f, sp->f3val[2]);
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

    parser.RegisterObjectType(Simple::GetObjectSpec());
    parser.RegisterObjectType(Derived::GetObjectSpec());

    Parser::ObjectPtr obj = parser.ParseString(input);
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
