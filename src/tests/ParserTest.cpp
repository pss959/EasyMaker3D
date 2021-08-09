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

TEST_F(ParserTest, StreamAndFile) {
    std::vector<Parser::FieldSpec> specs{
        { "field1", Parser::ValueType::kInteger, 1 },
        { "field2", Parser::ValueType::kFloat,   3 },
    };

    const std::string input = "AnObj { field1: 13, field2: .1 2 3.4, }";

    InitStream(input);

    // Write the input to a temporary file as well.
    boost::filesystem::path path = boost::filesystem::temp_directory_path() /
        boost::filesystem::unique_path();
    std::string path_str = path.native();
    {
        std::ofstream out(path_str);
        out << input << "\n";
    }

    // Parse both and test the results.
    Parser::Parser parser(specs);
    Parser::ObjectPtr root1 = parser.ParseStream(in);
    Parser::ObjectPtr root2 = parser.ParseFile(path_str);

    boost::filesystem::remove(path);

    for (Parser::ObjectPtr root: { root1, root2 }) {

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
        TEST_THROW_(root->fields[0]->GetValue<float>(), "Invalid type");
        TEST_THROW_(root->fields[0]->GetValues<int>(),  "Attempt to GetValues");
    }
}

TEST_F(ParserTest, ZeroCount) {
    std::vector<Parser::FieldSpec> specs{
        { "field1", Parser::ValueType::kInteger, 0 },
    };
    TEST_THROW_(Parser::Parser parser(specs), "invalid count");
}

TEST_F(ParserTest, SpecConflict) {
    {
        // Type conflict.
        std::vector<Parser::FieldSpec> specs{
            { "field1", Parser::ValueType::kInteger, 1 },
            { "field2", Parser::ValueType::kFloat,   2 },
            { "field1", Parser::ValueType::kFloat,   1 },
        };
        TEST_THROW_(Parser::Parser parser(specs), "Conflicting types/counts");
    }
    {
        // Count conflict.
        std::vector<Parser::FieldSpec> specs{
            { "field1", Parser::ValueType::kInteger, 1 },
            { "field2", Parser::ValueType::kFloat,   2 },
            { "field1", Parser::ValueType::kInteger, 3 },
        };
        TEST_THROW_(Parser::Parser parser(specs), "Conflicting types/counts");
    }
}

TEST_F(ParserSyntaxTest, SyntaxErrors) {
    InitStream("=");
    TEST_THROW_(parser->ParseStream(in), "Invalid empty type name");

    InitStream("1Obj");
    TEST_THROW_(parser->ParseStream(in), "Invalid type name");

    InitStream("Obj =");
    TEST_THROW_(parser->ParseStream(in), "Expected '{'");
}
