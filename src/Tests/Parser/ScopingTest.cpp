#include "Tests/Parser/ParserTestBase.h"

/// \ingroup Tests
class ScopingTest : public ParserTestBase {};

TEST_F(ScopingTest, Scoping) {
    const std::string input =
        "Derived \"D1\" {\n"             // dp1
        "  simple: Simple \"S1\" {},\n"  // sp1
        "  simple_list: [\n"
        "    USE \"S1\",\n"              // Should be instance of sp1
        "    Derived \"D2\" {\n"         // dp2
        "      simple_list: [\n"
        "        Simple \"S1\" {},\n"    // sp2
        "        USE \"S1\",\n"          // Should be instance of sp2
        "      ],\n"
        "    },\n"
        "    Derived \"D3\" {\n"         // dp3
        "      simple: USE \"S1\",\n"    // Should be instance of sp1
        "    },\n"
        "  ],\n"
        "}\n";

    Parser::ObjectPtr obj = ParseString(input);
    EXPECT_NOT_NULL(obj.get());
    DerivedPtr dp1 = std::dynamic_pointer_cast<Derived>(obj);
    EXPECT_NOT_NULL(dp1.get());

    const auto sp1 = dp1->simple.GetValue();
    const auto &list1 = dp1->simple_list.GetValue();
    EXPECT_EQ(3U, list1.size());
    EXPECT_EQ_OBJS(sp1, list1[0]);

    const auto dp2 = std::dynamic_pointer_cast<Derived>(list1[1]);
    EXPECT_NOT_NULL(dp2.get());
    const auto &list2 = dp2->simple_list.GetValue();
    EXPECT_EQ(2U, list2.size());
    const auto sp2 = list2[0];
    EXPECT_NE(sp1, sp2);
    EXPECT_EQ_OBJS(sp2, list2[1]);

    const auto dp3 = std::dynamic_pointer_cast<Derived>(list1[2]);
    EXPECT_NOT_NULL(dp3.get());
    EXPECT_EQ_OBJS(sp1, dp3->simple.GetValue());
}
