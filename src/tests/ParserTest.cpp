#include <memory>
#include <string>
#include <sstream>
#include <vector>

#include "Parser/Parser.h"
#include "Testing.h"

// Tests that a Parser::Exception is thrown and that its message contains
// the given string.
#define TEST_THROW(stmt, pattern)                                             \
    EXPECT_THROW({                                                            \
            { try {                                                           \
                    stmt;                                                     \
              }                                                               \
              catch (const Parser::Exception &ex) {                           \
                  EXPECT_TRUE(std::string(ex.what()).find(pattern) !=         \
                              std::string::npos) <<                           \
                      "Exception string:\n   " << ex.what()                   \
                                            << "\nvs. pattern: " << pattern;  \
                  throw;                                                      \
              }                                                               \
            }                                                                 \
        }, Parser::Exception)

class ParserTest : public ::testing::Test {
 protected:
    std::istringstream in;
    void InitStream(const std::string &input_string) {
        in.clear();
        in.str(input_string);
    }
};

class ParserSyntaxTest : public ParserTest {
 protected:
    virtual void SetUp() override {
        static std::vector<Parser::FieldSpec> specs{
            { "field1", Parser::ValueType::kInteger, 1 },
            { "field2", Parser::ValueType::kFloat,   3 },
        };
        parser.reset(new Parser::Parser(specs));
    }
    std::unique_ptr<Parser::Parser> parser;
};

TEST_F(ParserTest, RealData) {
    std::vector<Parser::FieldSpec> specs{
        { "field1", Parser::ValueType::kInteger, 1 },
        { "field2", Parser::ValueType::kFloat,   3 },
    };

    InitStream("AnObj { field1: 13, field2: .1 2 3.4, }");

    Parser::Parser parser(specs);
    Parser::ObjectPtr root = parser.ParseStream(in);
    EXPECT_NOT_NULL(root.get());
    EXPECT_EQ("AnObj", root->type_name);

    EXPECT_EQ(2U,       root->fields.size());
    EXPECT_EQ(specs[0], root->fields[0]->spec);
    EXPECT_EQ(specs[1], root->fields[1]->spec);
    EXPECT_EQ(13,       root->fields[0]->GetValue<int>());
    const std::vector<float> expected = std::vector<float>{ .1f, 2.f, 3.4f };
    const std::vector<float> actual   = root->fields[1]->GetValues<float>();
    EXPECT_EQ(expected, actual);

    // Check for exceptions for invalid accesses.
    TEST_THROW(root->fields[0]->GetValue<float>(), "Invalid type");
    TEST_THROW(root->fields[0]->GetValues<int>(),  "Attempt to GetValues");
}

TEST_F(ParserTest, ZeroCount) {
    std::vector<Parser::FieldSpec> specs{
        { "field1", Parser::ValueType::kInteger, 0 },
    };
    TEST_THROW(Parser::Parser parser(specs), "invalid count");
}

TEST_F(ParserTest, SpecConflict) {
    {
        // Type conflict.
        std::vector<Parser::FieldSpec> specs{
            { "field1", Parser::ValueType::kInteger, 1 },
            { "field2", Parser::ValueType::kFloat,   2 },
            { "field1", Parser::ValueType::kFloat,   1 },
        };
        TEST_THROW(Parser::Parser parser(specs), "Conflicting types/counts");
    }
    {
        // Count conflict.
        std::vector<Parser::FieldSpec> specs{
            { "field1", Parser::ValueType::kInteger, 1 },
            { "field2", Parser::ValueType::kFloat,   2 },
            { "field1", Parser::ValueType::kInteger, 3 },
        };
        TEST_THROW(Parser::Parser parser(specs), "Conflicting types/counts");
    }
}

TEST_F(ParserSyntaxTest, SyntaxErrors) {
    InitStream("=");
    TEST_THROW(parser->ParseStream(in), "Invalid empty type name");

    InitStream("1Obj");
    TEST_THROW(parser->ParseStream(in), "Invalid type name");

    InitStream("Obj =");
    TEST_THROW(parser->ParseStream(in), "Expected '{'");
}
