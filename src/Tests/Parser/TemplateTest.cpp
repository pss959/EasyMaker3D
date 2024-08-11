//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Tests/Parser/ParserTestBase.h"

/// \ingroup Tests
class TemplateTest : public ParserTestBase {};

TEST_F(TemplateTest, Templates) {
    const Str input =
        "Derived {\n"
        "  TEMPLATES: [\n"
        "    Simple \"T_1\" {\n"
        "      int_val:   32,\n"
        "      float_val: 12.5,\n"
        "    },\n"
        "    Simple \"T_2\" {\n"
        "      int_val:   14,\n"
        "      float_val: 21.5,\n"
        "    },\n"
        "  ],\n"
        "  simple_list: [\n"
        // This clone should use both template values.
        "    CLONE \"T_1\" \"Inst1\" {},\n"
        // This clone should override int_val.
        "    CLONE \"T_2\" \"Inst2\" {\n"
        "      int_val: 271,\n"
        "    },\n"
        "  ],\n"
        "}\n";

    Parser::ObjectPtr obj = ParseString(input);
    EXPECT_NOT_NULL(obj.get());
    DerivedPtr dp = std::dynamic_pointer_cast<Derived>(obj);
    EXPECT_NOT_NULL(dp.get());

    // Validate the clones.
    const std::vector<SimplePtr> &list = dp->simple_list;
    EXPECT_EQ(2U, list.size());
    const SimplePtr inst1 = list[0];
    const SimplePtr inst2 = list[1];
    EXPECT_EQ("Inst1", inst1->GetName());
    EXPECT_EQ(32,   inst1->int_val);
    EXPECT_EQ(12.5, inst1->float_val);
    EXPECT_EQ("Inst2", inst2->GetName());
    EXPECT_EQ(271,  inst2->int_val);    // Override template value.
    EXPECT_EQ(21.5, inst2->float_val);  // Inherit template value.
    EXPECT_TRUE(inst1->IsClone());
    EXPECT_TRUE(inst2->IsClone());

    const Str bad_input =
        "Derived {\n"
        "  int_val: 12,\n"
        "  TEMPLATES: [ Simple \"TempName\" {} ],\n"
        "}\n";
    TEST_THROW_(parser.ParseFromString(bad_input),
                "TEMPLATES appears after fields");
}
