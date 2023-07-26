#include "ParserTestBase.h"

// ----------------------------------------------------------------------------
// Parser error tests. These do not use the ParserTestBase functions that catch
// exceptions.
// ----------------------------------------------------------------------------

class ErrorTest : public ParserTestBase {};

TEST_F(ErrorTest, BadFile) {
    TEST_THROW_(parser.ParseFile("/no/such/file/exists"),
                "Failed to open file");
}

TEST_F(ErrorTest, ObjectTypeConflict) {
    InitSimple();
    TEST_THROW_(InitSimple(), "Object type registered more than once");
}

TEST_F(ErrorTest, BadReference) {
    TempFile file1("");
    TempFile file2("Simple \"Child2\" {}");
    const std::string input =
        "Derived { \n"
        "  simple_list: [\n"
        "      Simple \"Child1\" {},\n"
        "      USE \"Child2\",\n"  // Bad reference.
        "  ],\n"
        "}\n";
    InitDerived();
    TEST_THROW_(parser.ParseFromString(input), "Missing object");
}

TEST_F(ErrorTest, SyntaxErrors) {
    InitDerived();
    Parser::Registry::AddType<Other>("Other");

    TEST_THROW_(parser.ParseFromString(" "),
                "Invalid empty name for object type");
    TEST_THROW_(parser.ParseFromString(" 01BadName {}"),
                "Invalid name");
    TEST_THROW_(parser.ParseFromString("Simplex"),
                "Unknown object type");
    TEST_THROW_(parser.ParseFromString("Simple ="),
                "Expected '{'");
    TEST_THROW_(parser.ParseFromString("Simple { bool_val: z }"),
                "Invalid bool value");
    TEST_THROW_(parser.ParseFromString("Simple { bool_val: tralse }"),
                "Invalid bool value");
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
    TEST_THROW_(parser.ParseFromString("Simple { uint_val: 0x12345667875675 }"),
                "Invalid unsigned integer value");
    TEST_THROW_(parser.ParseFromString("Simple { str_val: \""),
                "Found EOF inside quoted string");
    TEST_THROW_(parser.ParseFromString("Simple { vec3f_val: 12 abc 4 }"),
                "Invalid float value");
    TEST_THROW_(parser.ParseFromString("Simple { vec3f_val: 12 abc 4 }"),
                "Invalid float value");
    TEST_THROW_(parser.ParseFromString("Simple { color_val: \"#badcolor\" }"),
                "Invalid color format");
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

    // Test nested file reporting using a temporary file that is included.
    TempFile included("Simple { bad_field: 12 }");
    TEST_THROW_(parser.ParseFromString(
                    "Derived { simple: <\"" +
                    included.GetPath().ToString() + "\"> }"),
                "Unknown field");
}
