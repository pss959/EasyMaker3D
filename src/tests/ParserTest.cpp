#include <string>
#include <sstream>
#include <vector>

#include "Parser.h"
#include "Testing.h"

TEST(Parser, XXXX) {
    std::vector<Parser::FieldSpec> specs{
        { "field1", Parser::ValueType::kInteger, 1 },
        { "field2", Parser::ValueType::kFloat,   3 },
    };

    std::string input = "AnObj { field1: 13, field2: .1 2 3.4, }";
    std::istringstream in(input);

    Parser parser(specs);
    Parser::ObjectPtr root = parser.ParseStream(in);
    EXPECT_NOT_NULL(root.get());
    EXPECT_EQ("AnObj", root->type_name);

    EXPECT_EQ(2U,       root->fields.size());
    EXPECT_EQ(specs[0], root->fields[0]->spec);
    EXPECT_EQ(specs[1], root->fields[1]->spec);
    EXPECT_EQ(13,       root->fields[0]->GetValue<int>());
    const std::vector<float> expected = std::vector<float>{ .1f, 2.f, 3.4f };
    const std::vector<float> actual   = root->fields[1]->GetValues<float, 3>();
    EXPECT_EQ(expected, actual);

    // XXXX THROWS! EXPECT_EQ(13,       root->fields[0]->GetValue<float>());
}
