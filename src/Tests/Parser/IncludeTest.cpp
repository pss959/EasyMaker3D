#include "ParserTestBase.h"

class IncludeTest : public ParserTestBase {};

TEST_F(IncludeTest, Includes) {
    TempFile file1("Simple \"Child1\" {}");
    TempFile file2("Simple \"Child2\" {}");
    const std::string input =
        "Derived \"ParentName\" { \n"
        "  simple_list: [\n"
        "      <\"" + file1.GetPath().ToString() + "\">,\n"
        "      <\"" + file2.GetPath().ToString() + "\">,\n"
        "  ],\n"
        "}\n";

    InitDerived();
    Parser::ObjectPtr obj = ParseString(input);
    EXPECT_NOT_NULL(obj.get());

    EXPECT_EQ("Derived", obj->GetTypeName());
    EXPECT_EQ("ParentName", obj->GetName());
    DerivedPtr dp = std::dynamic_pointer_cast<Derived>(obj);
    EXPECT_NOT_NULL(dp.get());
    const std::vector<SimplePtr> &list = dp->simple_list;
    EXPECT_FALSE(list.empty());
    EXPECT_EQ(2U, list.size());
    EXPECT_EQ("Child1", list[0]->GetName());
    EXPECT_EQ("Child2", list[1]->GetName());
}
