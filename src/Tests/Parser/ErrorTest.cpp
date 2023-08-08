#include "Tests/Parser/ParserTestBase.h"
#include "Tests/TempFile.h"

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
    TEST_THROW_(InitTestClasses(), "Object type registered more than once");
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
    TEST_THROW_(parser.ParseFromString(input), "Missing object");
}

TEST_F(ErrorTest, SyntaxErrors) {
    // Parse from a string after resetting the Parser to avoid pollution.
    auto parse_str = [&](const std::string &s){
        parser.Reset();
        return parser.ParseFromString(s);
    };

    TEST_THROW_(parse_str(" "),
                "Invalid empty name for object type");
    TEST_THROW_(parse_str(" 01BadName {}"),
                "Invalid name");
    TEST_THROW_(parse_str("Simplex"),
                "Unknown object type");
    TEST_THROW_(parse_str("Simple ="),
                "Expected '{'");
    TEST_THROW_(parse_str("Simple { bool_val: z }"),
                "Invalid bool value");
    TEST_THROW_(parse_str("Simple { bool_val: tralse }"),
                "Invalid bool value");
    TEST_THROW_(parse_str("Simple { int_val: 9 x }"),
                "Expected ',' or '}'");
    TEST_THROW_(parse_str("Simple { int_val: b }"),
                "Invalid integer value");
    TEST_THROW_(parse_str("Simple { int_val: 123b }"),
                "Invalid integer value");
    TEST_THROW_(parse_str("Simple { int_val: 0xa1 }"),
                "Invalid integer value");
    TEST_THROW_(parse_str("Simple { uint_val: -12 }"),
                "Invalid unsigned integer value");
    TEST_THROW_(parse_str("Simple { uint_val: +4 }"),
                "Invalid unsigned integer value");
    TEST_THROW_(parse_str("Simple { uint_val: 0xqb }"),
                "Invalid unsigned integer value");
    TEST_THROW_(parse_str("Simple { uint_val: 0x12345667875675 }"),
                "Invalid unsigned integer value");
    TEST_THROW_(parse_str("Simple { str_val: \""),
                "Found EOF inside quoted string");
    TEST_THROW_(parse_str("Simple { vec3f_val: 12 abc 4 }"),
                "Invalid float value");
    TEST_THROW_(parse_str("Simple { vec3f_val: 12 abc 4 }"),
                "Invalid float value");
    TEST_THROW_(parse_str("Simple { color_val: \"#badcolor\" }"),
                "Invalid color format");
    TEST_THROW_(parse_str("Simple { enum_val: \"glorp\" }"),
                "Invalid value for enum");
    TEST_THROW_(parse_str("Simple { flag_val: \"glorp\" }"),
                "Invalid value for flag enum");
    TEST_THROW_(parse_str("Simple { flag_val: \"kF1|x\" }"),
                "Invalid value for flag enum");
    TEST_THROW_(parse_str("Simple"),
                "EOF");
    TEST_THROW_(parse_str("USE \"\""),
                "Missing Object name for USE");
    TEST_THROW_(parse_str("Derived \"Foo\" { simple: USE \"\" }"),
                "Missing Object name for USE");
    TEST_THROW_(parse_str("CLONE \"\""),
                "Missing Template or Object name for CLONE");
    TEST_THROW_(parse_str("CLONE \"NoSuchObj\""),
                "Missing Template or Object with name");
    TEST_THROW_(parse_str("Simple { bad_field: 13 }"),
                "Unknown field");
    TEST_THROW_(parse_str("Unscoped {}"),
                "must have a name");
    TEST_THROW_(parse_str("Unscoped \"A\" { CONSTANTS: [] }"),
                "CONSTANTS appears in unscoped object");
    TEST_THROW_(parse_str("Unscoped \"B\" { TEMPLATES: [] }"),
                "TEMPLATES appears in unscoped object");
    // Name causes Unscoped::IsValid() to return false.
    TEST_THROW_(parse_str("Unscoped \"INVALID\" {}"),
                "has error: invalid name");
    TEST_THROW_(parse_str("<\"include/with/eof\""),
                "Expected '>', got EOF");
    TEST_THROW_(parse_str("<\"\">"),
                "Invalid empty path");
    TEST_THROW_(parse_str("Derived { simple: Other {} }"),
                "Incorrect object type");
    TEST_THROW_(parse_str("Derived { simple_list: [Other {}] }"),
                "Incorrect object type");

    // Test nested file reporting using a temporary file that is included.
    TempFile included("Simple { bad_field: 12 }");
    TEST_THROW_(parse_str("Derived { simple: <\"" +
                          included.GetPath().ToString() + "\"> }"),
                "Unknown field");
}
