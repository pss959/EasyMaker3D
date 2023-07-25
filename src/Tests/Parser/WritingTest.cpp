#include "ParserTestBase.h"

#include "Parser/Writer.h"

class WritingTest : public ParserTestBase {};

TEST_F(WritingTest, Writer) {
    InitDerived();

    auto dp  = Parser::Registry::CreateObject<Derived>();
    auto sp0 = Parser::Registry::CreateObject<Simple>();
    auto sp1 = Parser::Registry::CreateObject<Simple>();
    auto sp2 = Parser::Registry::CreateObject<Simple>();

    Util::Flags<FlagEnum> flg;
    flg.Set(FlagEnum::kF2);
    const Vector3f  vec(3, 4, 5);
    const Color     col(.2f, 1, .5f, 1);
    const Anglef    ang = Anglef::FromDegrees(110);
    const Rotationf rot = Rotationf::FromAxisAndAngle(Vector3f(1, 0, 0),
                                                      Anglef::FromDegrees(30));

    sp0->int_val   = 15;
    sp1->float_val = 2.5f;
    sp2->str_val   = "Ha!";

    dp->bool_val    = true;
    dp->int_val     = -13;
    dp->uint_val    = 123;
    dp->float_val   = 6.5f;
    dp->str_val     = "A string";
    dp->enum_val    = SimpleEnum::kE2;
    dp->flag_val    = flg;
    dp->vec3f_val   = vec;
    dp->color_val   = col;
    dp->angle_val   = ang;
    dp->rot_val     = rot;
    dp->ints_val    = std::vector<int>{ 3, -2 };
    dp->strs_val    = std::vector<std::string>{ "Hello", "There" };
    dp->simple      = sp0;
    dp->simple_list = std::vector<SimplePtr>{ sp1, sp2 };
    dp->hidden_int  = 6;

    std::ostringstream out;
    {
        Parser::Writer writer(out);
        writer.WriteObject(*dp);
    }

    const std::string kExpected =
        "Derived {\n"
        "  bool_val: True,\n"
        "  int_val: -13,\n"
        "  uint_val: 123,\n"
        "  float_val: 6.5,\n"
        "  str_val: \"A string\",\n"
        "  enum_val: \"kE2\",\n"
        "  flag_val: \"kF2\",\n"
        "  vec3f_val: 3 4 5,\n"
        "  color_val: 0.2 1 0.5 1,\n"
        "  angle_val: 110,\n"
        "  rot_val: 1 0 0 30,\n"
        "  ints_val: [3, -2],\n"
        "  strs_val: [\"Hello\", \"There\"],\n"
        "  simple: Simple {\n"
        "    int_val: 15,\n"
        "  },\n"
        "  simple_list: [\n"
        "    Simple {\n"
        "      float_val: 2.5,\n"
        "    },\n"
        "    Simple {\n"
        "      str_val: \"Ha!\",\n"
        "    },\n"
        "  ],\n"
        "}\n";

    EXPECT_TRUE(CompareStrings(kExpected, out.str()));
}
