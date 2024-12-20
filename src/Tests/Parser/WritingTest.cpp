//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Parser/Writer.h"
#include "Tests/Parser/ParserTestBase.h"

/// \ingroup Tests
class WritingTest : public ParserTestBase {};

TEST_F(WritingTest, WriteDerived) {
    auto dp  = Parser::Registry::CreateObject<Derived>();
    auto sp0 = Parser::Registry::CreateObject<Simple>();
    auto sp1 = Parser::Registry::CreateObject<Simple>();
    auto sp2 = Parser::Registry::CreateObject<Simple>();

    Util::Flags<FlagEnum> flags;
    flags.Set(FlagEnum::kF2);
    const Vector3f  vec(3, 4, 5);
    const Color     col(.2f, 1, .5f, 1);
    const Anglef    ang = Anglef::FromDegrees(110);
    const Rotationf rot = BuildRotation(1, 0, 0, 30);

    sp0->int_val   = 15;
    sp1->float_val = 2.5f;
    sp2->str_val   = "Ha!";

    dp->bool_val    = true;
    dp->int_val     = -13;
    dp->uint_val    = 123;
    dp->float_val   = 6.5f;
    dp->str_val     = "A string";
    dp->enum_val    = SimpleEnum::kE2;
    dp->flag_val    = flags;
    dp->vec3f_val   = vec;
    dp->color_val   = col;
    dp->angle_val   = ang;
    dp->rot_val     = rot;
    dp->ints_val    = std::vector<int>{ 3, -2 };
    dp->strs_val    = StrVec{ "Hello", "There" };
    dp->simple      = sp0;
    dp->simple_list = std::vector<SimplePtr>{ sp1, sp2 };
    dp->hidden_int  = 6;

    std::ostringstream out;
    {
        Parser::Writer writer(out);
        writer.WriteComment("A header comment");
        writer.WriteObject(*dp);
    }

    const Str kExpected =
        "# A header comment\n"
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

TEST_F(WritingTest, WriteFull) {
    auto fp = Parser::Registry::CreateObject<Full>();

    Util::Flags<FlagEnum> flags;
    flags.Set(FlagEnum::kF3);
    flags.Set(FlagEnum::kF2);

    fp->b    = true;
    fp->i    = -13;
    fp->u    = 123;
    fp->z    = 567;
    fp->f    = 6.5f;
    fp->s    = "A string";
    fp->e    = SimpleEnum::kE2;
    fp->g    = flags;
    fp->v2f  = Vector2f(-3.5f, 4);
    fp->v3f  = Vector3f(-3.5f, 4, 5);
    fp->v4f  = Vector4f(-3.5f, 4, 5, 6);
    fp->v2i  = Vector2i(-3, 4);
    fp->v3i  = Vector3i(-3, 4, 5);
    fp->v4i  = Vector4i(-3, 4, 5, 6);
    fp->v2ui = Vector2ui(3, 4);
    fp->v3ui = Vector3ui(3, 4, 5);
    fp->v4ui = Vector4ui(3, 4, 5, 6);
    fp->p2f  = Point2f(-3.5f, 4);
    fp->p3f  = Point3f(-3.5f, 4, 5);
    fp->p2ui = Point2ui(3, 4);
    fp->c    = Color(.2f, 1, .5f, 1);
    fp->a    = Anglef::FromDegrees(-40);
    fp->r    = BuildRotation(1, 0, 0, 30);
    fp->m2   = Matrix2f(1, 2, 3, 4);
    fp->m3   = Matrix3f(1, 2, 3, 4, 5, 6, 7, 8, 9);
    fp->m4   = Matrix4f(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
    fp->pl   = Plane(-4.5f, Vector3f(0, 1, 0));
    fp->ca   = CircleArc(Anglef::FromDegrees(15), Anglef::FromDegrees(-250));

    std::ostringstream out;
    {
        Parser::Writer writer(out);
        writer.WriteObject(*fp);
    }

    const Str kExpected =
        "Full {\n"
        "  b: True,\n"
        "  i: -13,\n"
        "  u: 123,\n"
        "  z: 567,\n"
        "  f: 6.5,\n"
        "  s: \"A string\",\n"
        "  e: \"kE2\",\n"
        "  g: \"kF2|kF3\",\n"
        "  v2f: -3.5 4,\n"
        "  v3f: -3.5 4 5,\n"
        "  v4f: -3.5 4 5 6,\n"
        "  v2i: -3 4,\n"
        "  v3i: -3 4 5,\n"
        "  v4i: -3 4 5 6,\n"
        "  v2ui: 3 4,\n"
        "  v3ui: 3 4 5,\n"
        "  v4ui: 3 4 5 6,\n"
        "  p2f: -3.5 4,\n"
        "  p3f: -3.5 4 5,\n"
        "  p2ui: 3 4,\n"
        "  c: 0.2 1 0.5 1,\n"
        "  a: -40,\n"
        "  r: 1 0 0 30,\n"
        "  m2: 1 2 3 4,\n"
        "  m3: 1 2 3 4 5 6 7 8 9,\n"
        "  m4: 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16,\n"
        "  pl: 0 1 0 -4.5,\n"
        "  ca: 15 -250,\n"
        "}\n";

    EXPECT_TRUE(CompareStrings(kExpected, out.str()));
}

TEST_F(WritingTest, ObjectFunc) {
    auto dp  = Parser::Registry::CreateObject<Derived>();
    auto sp1 = Parser::Registry::CreateObject<Simple>("SP1");
    auto sp2 = Parser::Registry::CreateObject<Simple>("SP2");
    auto sp3 = Parser::Registry::CreateObject<Simple>("SP3");
    dp->simple_list = std::vector<SimplePtr>{ sp1, sp2, sp3 };

    bool called_before = false;
    bool called_after  = false;

    // Set a function that filters out only sp2.
    const auto obj_func = [&](const Parser::Object &obj, bool is_before){
        if (is_before)
            called_before = true;
        else
            called_after  = true;
        return &obj != sp2.get();
    };

    std::ostringstream out;
    {
        Parser::Writer writer(out);
        writer.WriteObjectConditional(*dp, obj_func);
    }

    const Str kExpected =
        "Derived {\n"
        "  simple_list: [\n"
        "    Simple \"SP1\" {\n"
        "    },\n"
        "    Simple \"SP3\" {\n"
        "    },\n"
        "  ],\n"
       "}\n";

    EXPECT_TRUE(CompareStrings(kExpected, out.str()));
    EXPECT_TRUE(called_before);
    EXPECT_TRUE(called_after);
}

TEST_F(WritingTest, Addresses) {
    auto dp = Parser::Registry::CreateObject<Derived>();
    auto sp = Parser::Registry::CreateObject<Simple>("SP1");
    dp->simple = sp;
    dp->simple_list = std::vector<SimplePtr>{ sp };  // Creates a USE.

    const auto daddr = Util::ToString(dp.get());
    const auto saddr = Util::ToString(sp.get());

    std::ostringstream out;
    {
        Parser::Writer writer(out);
        writer.SetAddressFlag(true);
        writer.WriteObject(*dp);
    }

    const Str kExpected =
        "Derived { # " + daddr + "\n"
        "  simple: Simple \"SP1\" { # " + saddr + "\n"
        "  },\n"
        "  simple_list: [\n"
        "    USE \"SP1\", # " + saddr + ",\n"
        "  ],\n"
       "}\n";

    EXPECT_TRUE(CompareStrings(kExpected, out.str()));
}

TEST_F(WritingTest, WriteField) {
    auto sp = Parser::Registry::CreateObject<Simple>();
    sp->int_val = 21;
    sp->str_val = "some text";

    {
        std::ostringstream out;
        {
            Parser::Writer writer(out);
            writer.WriteField(sp->int_val);
        }
        EXPECT_TRUE(CompareStrings("int_val: 21,\n", out.str()));
    }
    {
        std::ostringstream out;
        {
            Parser::Writer writer(out);
            writer.WriteField(sp->str_val);
        }
        EXPECT_TRUE(CompareStrings("str_val: \"some text\",\n", out.str()));
    }
}
