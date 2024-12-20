//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Parser/Parser.h"
#include "Math/Types.h"
#include "SG/ColorMap.h"
#include "Tests/TestBaseWithTypes.h"
#include "Tests/Testing2.h"
#include "Util/Assert.h"
#include "Util/General.h"

/// \ingroup Tests
class ColorMapTest : public TestBaseWithTypes {};

TEST_F(ColorMapTest, Default) {
    // No instance => returns white.
    SG::ColorMap::Reset();
    EXPECT_EQ(Color::White(), SG::ColorMap::SGetColor("noname"));

    // No instance => assertion failure.
    TEST_ASSERT(SG::ColorMap::SGetColorForDimension(1), "instance");

    auto cm = CreateObject<SG::ColorMap>();

    EXPECT_TRUE(cm->GetNamedColors().empty());

    TEST_ASSERT(cm->GetColor("badname"),                "Missing color");
    TEST_ASSERT(cm->GetColorForDimension(0),            "Missing color");

    TEST_ASSERT(SG::ColorMap::SGetColor("noname"),      "Missing color");
    TEST_ASSERT(SG::ColorMap::SGetColorForDimension(1), "Missing color");
}

TEST_F(ColorMapTest, Resource) {
    // Read a ColorMap with required dimension colors and a few others.
    const Str input =
        "ColorMap {\n"
        "  named_colors: [\n"
        "    NamedColor \"DimensionColorX\" { color: 1 0 0 1 }\n"
        "    NamedColor \"DimensionColorY\" { color: 0 1 0 1 }\n"
        "    NamedColor \"DimensionColorZ\" { color: 0 0 1 1 }\n"
        "    NamedColor \"TestColor1\"      { color: 1 0 1 1 }\n"
        "    NamedColor \"TestColor2\"      { color: 0 1 1 1 }\n"
        "  ]\n"
        "}\n";

    Parser::Parser parser;
    auto obj = parser.ParseFromString(input);

    EXPECT_TRUE(Util::IsA<SG::ColorMap>(obj));
    auto cm = std::dynamic_pointer_cast<SG::ColorMap>(obj);
    EXPECT_NOT_NULL(cm.get());

    EXPECT_EQ(5U, cm->GetNamedColors().size());
    EXPECT_EQ(Color(1, 0, 0, 1), cm->GetColorForDimension(0));
    EXPECT_EQ(Color(0, 1, 0, 1), cm->GetColorForDimension(1));
    EXPECT_EQ(Color(0, 0, 1, 1), cm->GetColorForDimension(2));
    EXPECT_EQ(Color(1, 0, 1, 1), cm->GetColor("TestColor1"));
    EXPECT_EQ(Color(0, 1, 1, 1), cm->GetColor("TestColor2"));

    EXPECT_EQ(Color(1, 0, 0, 1), SG::ColorMap::SGetColorForDimension(0));
    EXPECT_EQ(Color(0, 1, 0, 1), SG::ColorMap::SGetColorForDimension(1));
    EXPECT_EQ(Color(0, 0, 1, 1), SG::ColorMap::SGetColorForDimension(2));
    EXPECT_EQ(Color(1, 0, 1, 1), SG::ColorMap::SGetColor("TestColor1"));
    EXPECT_EQ(Color(0, 1, 1, 1), SG::ColorMap::SGetColor("TestColor2"));
}
