#include "Tests/Parser/ParserTestBase.h"

class ConstantTest : public ParserTestBase {};

TEST_F(ConstantTest, Constants) {
    const std::string input =
        "Simple {\n"
        "  CONSTANTS: [\n"
        "     FOO: \"123\",\n"
        "     BAR: \"2.5 $FOO 5.0\",\n"
        "     FOO_0: \"456\",\n"
        "  ],\n"
        "  int_val:  $FOO,\n"
        "  vec3f_val: $BAR,\n"
        "  float_val: $FOO_0,\n"
        "}\n";
    Parser::ObjectPtr obj = ParseString(input);
    EXPECT_NOT_NULL(obj);
    EXPECT_EQ("Simple",  obj->GetTypeName());
    SimplePtr sp = std::dynamic_pointer_cast<Simple>(obj);
    EXPECT_NOT_NULL(sp.get());
    EXPECT_EQ(123, sp->int_val);
    EXPECT_EQ(Vector3f(2.5f, 123.f, 5.f), sp->vec3f_val);
    EXPECT_EQ(456.f, sp->float_val);

    const std::string bad_input =
        "Simple {\n"
        "  int_val: 12,\n"
        "  CONSTANTS: [ FOO: \"123\" ],\n"
        "}\n";
    TEST_THROW_(parser.ParseFromString(bad_input),
                "CONSTANTS appears after fields");

    TEST_THROW_(parser.ParseFromString("Simple { $FOO }"),
                "Missing constant with name");
}
