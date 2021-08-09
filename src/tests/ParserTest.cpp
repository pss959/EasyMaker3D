#include <memory>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>

#include <boost/filesystem.hpp>

#include "Parser/Parser.h"
#include "Testing.h"

// Tests that a Parser::Exception is thrown and that its message contains
// the given string pattern.
#define TEST_THROW_(STMT, PATTERN) \
    TEST_THROW(STMT, Parser::Exception, PATTERN)

class ParserTest : public TestBase {
 protected:
    // Handy basic specs to use.
    std::vector<Parser::ObjectSpec> basic_specs{
        { "AnObj", {
                { "field1", Parser::ValueType::kInteger   },
                { "field2", Parser::ValueType::kFloat,  3 },
                { "field3", Parser::ValueType::kBool, },
          }
        }
    };

    // Handy specs with all types.
    std::vector<Parser::ObjectSpec> full_specs{
        { "ParentObj", {
                { "bool",   Parser::ValueType::kBool        },
                { "int",    Parser::ValueType::kInteger     },
                { "vec3f",  Parser::ValueType::kFloat,    3 },
                { "string", Parser::ValueType::kString,     },
                { "object", Parser::ValueType::kObject,     },
                { "list",   Parser::ValueType::kObjectList, }
            }},
        { "ChildObj", {
                { "int",    Parser::ValueType::kInteger     }
            }}
    };

    // Sets up a stream with an input string.
    std::istringstream in;
    void InitStream(const std::string &input_string) {
        in.clear();
        in.str(input_string);
    }
};

class ParserSyntaxTest : public ParserTest {
 protected:
    virtual void SetUp() override {
        parser.reset(new Parser::Parser(basic_specs));
    }
    std::unique_ptr<Parser::Parser> parser;
};

TEST_F(ParserTest, StreamAndFile) {
    const std::string input =
        "# Full-line comment\n"
        "AnObj {\n"
        "  field1: 13, # In-line comment\n"
        "  field2: .1 2 3.4,\n"
        "}\n";

    // Set up a stream and a temporary file with the input string.
    InitStream(input);
    TempFile tmp_file(input);

    // Parse both and test the results.
    Parser::Parser parser(basic_specs);
    Parser::ObjectPtr root1 = parser.ParseStream(in);
    Parser::ObjectPtr root2 = parser.ParseFile(tmp_file.GetPathString());

    for (Parser::ObjectPtr root: { root1, root2 }) {
        EXPECT_NOT_NULL(root.get());
        EXPECT_EQ("AnObj",                     root->spec.type_name);
        EXPECT_EQ(2U,                          root->fields.size());
        EXPECT_EQ("field1",                    root->fields[0]->spec.name);
        EXPECT_EQ(Parser::ValueType::kInteger, root->fields[0]->spec.type);
        EXPECT_EQ(1U,                          root->fields[0]->spec.count);
        EXPECT_EQ("field2",                    root->fields[1]->spec.name);
        EXPECT_EQ(Parser::ValueType::kFloat,   root->fields[1]->spec.type);
        EXPECT_EQ(3U,                          root->fields[1]->spec.count);

        EXPECT_EQ(13, root->fields[0]->GetValue<int>());
        const std::vector<float> expected = std::vector<float>{ .1f, 2.f, 3.4f };
        const std::vector<float> actual   = root->fields[1]->GetValues<float>();
        EXPECT_EQ(expected, actual);

        // Check for exceptions for invalid accesses.
        TEST_THROW_(root->fields[0]->GetValue<float>(), "Invalid type");
        TEST_THROW_(root->fields[1]->GetValues<int>(),  "Invalid type");
        TEST_THROW_(root->fields[0]->GetValues<int>(),  "Attempt to GetValues");
        TEST_THROW_(root->fields[1]->GetValue<float>(), "Attempt to GetValue");
    }
}

TEST_F(ParserTest, AllTypes) {
    const std::string input =
        "ParentObj { \n"
        "  bool:    True,\n"
        "  int:     31,\n"
        "  vec3f:   .2 .3 4,\n"
        "  string:  \"Some String\",\n"
        "  object:  ChildObj { int: 14 },\n"
        "  list:    [\n"
        "      ChildObj { int: 21 },\n"
        "      ChildObj { int: 22 },\n"
        "      ChildObj { int: 23 },\n"
        "  ],\n"
        "}\n";

    InitStream(input);
    Parser::Parser parser(full_specs);
    Parser::ObjectPtr root = parser.ParseStream(in);

    EXPECT_NOT_NULL(root.get());
    EXPECT_EQ("ParentObj", root->spec.type_name);
    EXPECT_EQ(true, root->fields[0]->GetValue<bool>());
    EXPECT_EQ(31,   root->fields[1]->GetValue<int>());
    const std::vector<float> expected = std::vector<float>{ .2f, .3f, 4.f };
    const std::vector<float> actual   = root->fields[2]->GetValues<float>();
    EXPECT_EQ(expected, actual);
    EXPECT_EQ("Some String", root->fields[3]->GetValue<std::string>());

    Parser::ObjectPtr child = root->fields[4]->GetValue<Parser::ObjectPtr>();
    EXPECT_NOT_NULL(child);
    EXPECT_EQ("ChildObj", child->spec.type_name);
    EXPECT_EQ(14, child->fields[0]->GetValue<int>());

    const std::vector<Parser::ObjectPtr> list_kids =
        root->fields[5]->GetValue<std::vector<Parser::ObjectPtr>>();
    EXPECT_EQ(3U, list_kids.size());
    EXPECT_NOT_NULL(list_kids[0]);
    EXPECT_NOT_NULL(list_kids[1]);
    EXPECT_NOT_NULL(list_kids[2]);
    EXPECT_EQ("ChildObj", list_kids[0]->spec.type_name);
    EXPECT_EQ("ChildObj", list_kids[1]->spec.type_name);
    EXPECT_EQ("ChildObj", list_kids[2]->spec.type_name);
    EXPECT_EQ(21, list_kids[0]->fields[0]->GetValue<int>());
    EXPECT_EQ(22, list_kids[1]->fields[0]->GetValue<int>());
    EXPECT_EQ(23, list_kids[2]->fields[0]->GetValue<int>());
}

TEST_F(ParserTest, OverwriteField) {
    const std::string input =
        "AnObj { \n"
        "  field1: 10,\n"
        "  field1: 20,\n"
        "}";
    InitStream(input);

    Parser::Parser parser(basic_specs);
    Parser::ObjectPtr root = parser.ParseStream(in);
    EXPECT_EQ(2U, root->fields.size());
    EXPECT_EQ(10, root->fields[0]->GetValue<int>());
    EXPECT_EQ(20, root->fields[1]->GetValue<int>());
}

TEST_F(ParserTest, BoolParsing) {
    std::vector<Parser::ObjectSpec> specs{
        { "AnObj", {{ "bool", Parser::ValueType::kBool }}}};

    // All of these bool values should work.
    const std::string input =
        "AnObj { \n"
        "  bool: F,\n"
        "  bool: False,\n"
        "  bool: T,\n"
        "  bool: TruE,\n"
        "  bool: True,\n"
        "  bool: f,\n"
        "  bool: fAlSe,\n"
        "  bool: t,\n"
        "}";
    InitStream(input);

    Parser::Parser parser(specs);
    Parser::ObjectPtr root = parser.ParseStream(in);
    EXPECT_EQ(8U, root->fields.size());
}

TEST_F(ParserTest, NamedObjects) {
    const std::string input =
        "ParentObj \"ParentName\" { \n"
        "  list: [\n"
        "      ChildObj \"Child1\" {},\n"
        "      ChildObj \"Child2\" {},\n"
        "      ChildObj \"Child3\" {},\n"
        "      ChildObj \"Child2\";,\n"  // Reference
        "  ],\n"
        "}\n";

    InitStream(input);

    Parser::Parser parser(full_specs);
    Parser::ObjectPtr root = parser.ParseStream(in);
    const std::vector<Parser::ObjectPtr> kids =
        root->fields[0]->GetValue<std::vector<Parser::ObjectPtr>>();
    EXPECT_EQ(4U, kids.size());
    EXPECT_EQ(kids[1], kids[3]);  // Reference to same object ("Child2").
}

TEST_F(ParserTest, BadFile) {
    Parser::Parser parser(basic_specs);
    TEST_THROW_(parser.ParseFile("/no/such/file/exists"),
                "Failed to open file");
}

TEST_F(ParserTest, ObjectTypeConflict) {
    std::vector<Parser::ObjectSpec> specs{
        { "AnObj", {{ "field1",    Parser::ValueType::kInteger }}},
        { "AnObj", {{ "who_cares", Parser::ValueType::kFloat   }}}};
    TEST_THROW_(Parser::Parser parser(specs), "Multiple object specs");
}

TEST_F(ParserTest, ZeroCountError) {
    std::vector<Parser::ObjectSpec> specs{
        { "AnObj", {{ "field1", Parser::ValueType::kInteger, 0 }}}};
    TEST_THROW_(Parser::Parser parser(specs), "zero count");
}

TEST_F(ParserTest, MultipleObjectCountError) {
    std::vector<Parser::ObjectSpec> specs{
        { "AnObj", {{ "field1", Parser::ValueType::kObject, 2 }}}};
    TEST_THROW_(Parser::Parser parser(specs), "count > 1");
}

TEST_F(ParserTest, MultipleObjectListCountError) {
    std::vector<Parser::ObjectSpec> specs{
        { "AnObj", {{ "field1", Parser::ValueType::kObjectList, 2 }}}};
    TEST_THROW_(Parser::Parser parser(specs), "count > 1");
}

TEST_F(ParserTest, FieldNameConflict) {
    std::vector<Parser::ObjectSpec> specs{
        { "AnObj", {
                { "field1",    Parser::ValueType::kInteger },
                { "field2",    Parser::ValueType::kFloat },
                { "field1",    Parser::ValueType::kInteger }}}};
    TEST_THROW_(Parser::Parser parser(specs), "Multiple field specs");
}

TEST_F(ParserTest, BadReference) {
    const std::string input =
        "ParentObj \"ParentName\" { \n"
        "  list: [\n"
        "      ChildObj \"Child1\" {},\n"
        "      ChildObj \"Child2\";,\n"  // Bad reference
        "  ],\n"
        "}\n";

    InitStream(input);
    Parser::Parser parser(full_specs);
    TEST_THROW_(parser.ParseStream(in), "Invalid reference to object");
}

TEST_F(ParserSyntaxTest, SyntaxErrors) {
    InitStream("=");
    TEST_THROW_(parser->ParseStream(in), "Invalid empty type name");

    InitStream("1Obj");
    TEST_THROW_(parser->ParseStream(in), "Invalid type name");

    InitStream("AnObj =");
    TEST_THROW_(parser->ParseStream(in), "Expected '{'");

    InitStream("AnObj { field1: 9 x");
    TEST_THROW_(parser->ParseStream(in), "Expected ',' or '}'");

    InitStream("AnObj { field1: b");
    TEST_THROW_(parser->ParseStream(in), "Invalid integer value");

    InitStream("AnObj { field2: 12 abc 4");
    TEST_THROW_(parser->ParseStream(in), "Invalid float value");

    InitStream("AnObj { field3: \"glorp\" }");
    TEST_THROW_(parser->ParseStream(in), "Invalid bool value");

    InitStream("AnObj");
    TEST_THROW_(parser->ParseStream(in), "EOF");

    InitStream("BadObj");
    TEST_THROW_(parser->ParseStream(in), "Unknown object");

    InitStream("AnObj { bad_field: 13 }");
    TEST_THROW_(parser->ParseStream(in), "Unknown field");
}
