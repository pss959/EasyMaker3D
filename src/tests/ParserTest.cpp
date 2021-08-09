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
          }
        }
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
    const std::string input = "AnObj { field1: 13, field2: .1 2 3.4, }\n";

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
        TEST_THROW_(root->fields[0]->GetValues<int>(),  "Attempt to GetValues");
    }
}

TEST_F(ParserTest, AllTypes) {
    std::vector<Parser::ObjectSpec> specs{
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
    Parser::Parser parser(specs);
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

TEST_F(ParserTest, FieldNameConflict) {
    std::vector<Parser::ObjectSpec> specs{
        { "AnObj", {
                { "field1",    Parser::ValueType::kInteger },
                { "field2",    Parser::ValueType::kFloat },
                { "field1",    Parser::ValueType::kInteger }}}};
    TEST_THROW_(Parser::Parser parser(specs), "Multiple field specs");
}

TEST_F(ParserSyntaxTest, SyntaxErrors) {
    InitStream("=");
    TEST_THROW_(parser->ParseStream(in), "Invalid empty type name");

    InitStream("1Obj");
    TEST_THROW_(parser->ParseStream(in), "Invalid type name");

    InitStream("AnObj =");
    TEST_THROW_(parser->ParseStream(in), "Expected '{'");
}
