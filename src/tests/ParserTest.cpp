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

#if XXXX
TEST_F(ParserTest, AllTypes) {
    std::vector<Parser::FieldSpec> specs{
        { "bool",   Parser::ValueType::kBool        },
        { "int",    Parser::ValueType::kInteger     },
        { "vec3f",  Parser::ValueType::kFloat,    3 },
        { "string", Parser::ValueType::kString,     },
        { "object", Parser::ValueType::kObject,     },
        { "list",   Parser::ValueType::kObjectList, },
    };

    const std::string input =
        "Parent { field1: 13, field2: .1 2 3.4, }\n";

    // XXXX
}
#endif

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
