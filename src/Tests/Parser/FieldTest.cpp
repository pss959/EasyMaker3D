//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Tests/Parser/ParserTestBase.h"
#include "Tests/TempFile.h"

/// \ingroup Tests
class FieldTest : public ParserTestBase {};

TEST_F(FieldTest, Initialization) {
    auto dp = Parser::Registry::CreateObject<Derived>();

    // Test initial values.
    EXPECT_EQ(false,                   dp->bool_val.GetValue());
    EXPECT_EQ(0,                       dp->int_val.GetValue());
    EXPECT_EQ(0U,                      dp->uint_val.GetValue());
    EXPECT_EQ(0.f,                     dp->float_val.GetValue());
    EXPECT_EQ("",                      dp->str_val.GetValue());
    EXPECT_ENUM_EQ(SimpleEnum::kE1,    dp->enum_val.GetValue());
    EXPECT_EQ(Util::Flags<FlagEnum>(), dp->flag_val.GetValue());
    EXPECT_EQ(Vector3f::Zero(),        dp->vec3f_val.GetValue());
    EXPECT_EQ(Color::Black(),          dp->color_val.GetValue());
    EXPECT_EQ(Anglef(),                dp->angle_val.GetValue());
    EXPECT_EQ(Rotationf(),             dp->rot_val.GetValue());
    EXPECT_EQ(0U,                      dp->ints_val.GetValue().size());
    EXPECT_EQ(0U,                      dp->strs_val.GetValue().size());
    EXPECT_EQ(SimplePtr(),             dp->simple.GetValue());
    EXPECT_EQ(0U,                      dp->simple_list.GetValue().size());
    EXPECT_EQ(12,                      dp->hidden_int.GetValue());
}

TEST_F(FieldTest, AccessAndModify) {
    auto dp  = Parser::Registry::CreateObject<Derived>();
    auto sp0 = Parser::Registry::CreateObject<Simple>();
    auto sp1 = Parser::Registry::CreateObject<Simple>();
    auto sp2 = Parser::Registry::CreateObject<Simple>();

    // Test field setting and access.
    Util::Flags<FlagEnum> flg;
    flg.Set(FlagEnum::kF2);
    const Vector3f  vec(3, 4, 5);
    const Color     col(.2f, 1, .5f, 1);
    const Anglef    ang = Anglef::FromDegrees(110);
    const Rotationf rot = BuildRotation(1, 0, 0, 30);

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
    dp->strs_val    = StrVec{ "Hello", "There" };
    dp->simple      = sp0;
    dp->simple_list = std::vector<SimplePtr>{ sp1, sp2 };
    dp->hidden_int  = 6;

    EXPECT_EQ(true,                 dp->bool_val.GetValue());
    EXPECT_EQ(-13,                  dp->int_val.GetValue());
    EXPECT_EQ(123U,                 dp->uint_val.GetValue());
    EXPECT_EQ(6.5f,                 dp->float_val.GetValue());
    EXPECT_EQ("A string",           dp->str_val.GetValue());
    EXPECT_ENUM_EQ(SimpleEnum::kE2, dp->enum_val.GetValue());
    EXPECT_EQ(flg,                  dp->flag_val.GetValue());
    EXPECT_EQ(vec,                  dp->vec3f_val.GetValue());
    EXPECT_EQ(col,                  dp->color_val.GetValue());
    EXPECT_EQ(ang,                  dp->angle_val.GetValue());
    EXPECT_EQ(rot,                  dp->rot_val.GetValue());
    EXPECT_EQ(2U,                   dp->ints_val.GetValue().size());
    EXPECT_EQ(3,                    dp->ints_val.GetValue()[0]);
    EXPECT_EQ(-2,                   dp->ints_val.GetValue()[1]);
    EXPECT_EQ("Hello",              dp->strs_val.GetValue()[0]);
    EXPECT_EQ("There",              dp->strs_val.GetValue()[1]);
    EXPECT_EQ(6,                    dp->hidden_int.GetValue());

    // Test const field access and hidden flag.
    const Derived &cd = *dp;
    EXPECT_EQ(6, cd.hidden_int.GetValue());
    EXPECT_TRUE(cd.hidden_int.IsHidden());
    EXPECT_FALSE(cd.int_val.IsHidden());
}

TEST_F(FieldTest, ReplaceValue) {
    auto dp = Parser::Registry::CreateObject<Derived>();

    // Test field replacement.
    dp->ints_val = std::vector<int>{ 3, -2 };
    EXPECT_EQ(2U, dp->ints_val.GetValue().size());
    EXPECT_EQ(3,  dp->ints_val.GetValue()[0]);
    EXPECT_EQ(-2, dp->ints_val.GetValue()[1]);

    dp->ints_val.ReplaceValue(1, 13);
    EXPECT_EQ(2U, dp->ints_val.GetValue().size());
    EXPECT_EQ(3,  dp->ints_val.GetValue()[0]);
    EXPECT_EQ(13, dp->ints_val.GetValue()[1]);

    dp->ints_val.ReplaceValue(0, -8);
    EXPECT_EQ(2U, dp->ints_val.GetValue().size());
    EXPECT_EQ(-8, dp->ints_val.GetValue()[0]);
    EXPECT_EQ(13, dp->ints_val.GetValue()[1]);
}

TEST_F(FieldTest, WasSet) {
    const Str input =
        "Simple {\n"
        "  bool_val:  True,\n"
        "  float_val: 19,\n"
        "}\n";

    // Set up a temporary file with the input string.
    TempFile tmp_file(input);

    Parser::ObjectPtr obj = ParseString(input);
    EXPECT_NOT_NULL(obj.get());
    EXPECT_EQ("Simple",  obj->GetTypeName());
    SimplePtr sp = std::dynamic_pointer_cast<Simple>(obj);
    EXPECT_NOT_NULL(sp.get());
    EXPECT_TRUE(sp->bool_val.WasSet());
    EXPECT_TRUE(sp->float_val.WasSet());
    EXPECT_FALSE(sp->int_val.WasSet());
    EXPECT_FALSE(sp->uint_val.WasSet());
}

TEST_F(FieldTest, OverwriteField) {
    const Str input =
        "Simple {\n"
        "  int_val: 13,\n"
        "  int_val: 19,\n"
        "}\n";

    // Set up a temporary file with the input string.
    TempFile tmp_file(input);

    Parser::ObjectPtr obj = ParseString(input);
    EXPECT_NOT_NULL(obj.get());
    EXPECT_EQ("Simple",  obj->GetTypeName());
    SimplePtr sp = std::dynamic_pointer_cast<Simple>(obj);
    EXPECT_NOT_NULL(sp.get());
    EXPECT_EQ(19, sp->int_val);
}

TEST_F(FieldTest, EnumField) {
    auto sp = Parser::Registry::CreateObject<Simple>();

    sp->enum_val = SimpleEnum::kE2;
    EXPECT_EQ("E2", sp->enum_val.GetEnumWords());
}

TEST_F(FieldTest, FlagField) {
    auto sp = Parser::Registry::CreateObject<Simple>();

    EXPECT_FALSE(sp->flag_val.GetValue().Has(FlagEnum::kF1));
    EXPECT_FALSE(sp->flag_val.GetValue().Has(FlagEnum::kF2));
    EXPECT_FALSE(sp->flag_val.GetValue().Has(FlagEnum::kF3));

    sp->flag_val.Set(FlagEnum::kF2);
    EXPECT_FALSE(sp->flag_val.GetValue().Has(FlagEnum::kF1));
    EXPECT_TRUE(sp->flag_val.GetValue().Has(FlagEnum::kF2));
    EXPECT_FALSE(sp->flag_val.GetValue().Has(FlagEnum::kF3));

    sp->flag_val.Set(FlagEnum::kF3);
    EXPECT_FALSE(sp->flag_val.GetValue().Has(FlagEnum::kF1));
    EXPECT_TRUE(sp->flag_val.GetValue().Has(FlagEnum::kF2));
    EXPECT_TRUE(sp->flag_val.GetValue().Has(FlagEnum::kF3));

    sp->flag_val.Reset(FlagEnum::kF2);
    EXPECT_FALSE(sp->flag_val.GetValue().Has(FlagEnum::kF1));
    EXPECT_FALSE(sp->flag_val.GetValue().Has(FlagEnum::kF2));
    EXPECT_TRUE(sp->flag_val.GetValue().Has(FlagEnum::kF3));
}

TEST_F(FieldTest, ObjectListField) {
    auto dp  = Parser::Registry::CreateObject<Derived>();
    auto sp0 = Parser::Registry::CreateObject<Simple>();
    auto sp1 = Parser::Registry::CreateObject<Simple>();
    auto sp2 = Parser::Registry::CreateObject<Simple>();

    EXPECT_EQ(0U, dp->simple_list.GetValue().size());

    dp->simple_list.Add(sp0);
    EXPECT_EQ(1U,  dp->simple_list.GetValue().size());
    EXPECT_EQ(sp0, dp->simple_list.GetValue()[0]);

    dp->simple_list.Insert(0, sp1);
    EXPECT_EQ(2U,  dp->simple_list.GetValue().size());
    EXPECT_EQ(sp1, dp->simple_list.GetValue()[0]);
    EXPECT_EQ(sp0, dp->simple_list.GetValue()[1]);
    dp->simple_list.Insert(1, sp2);
    EXPECT_EQ(3U,  dp->simple_list.GetValue().size());
    EXPECT_EQ(sp1, dp->simple_list.GetValue()[0]);
    EXPECT_EQ(sp2, dp->simple_list.GetValue()[1]);
    EXPECT_EQ(sp0, dp->simple_list.GetValue()[2]);

    dp->simple_list.Remove(1);
    EXPECT_EQ(2U,  dp->simple_list.GetValue().size());
    EXPECT_EQ(sp1, dp->simple_list.GetValue()[0]);
    EXPECT_EQ(sp0, dp->simple_list.GetValue()[1]);

    dp->simple_list.Replace(0, sp2);
    EXPECT_EQ(2U,  dp->simple_list.GetValue().size());
    EXPECT_EQ(sp2, dp->simple_list.GetValue()[0]);
    EXPECT_EQ(sp0, dp->simple_list.GetValue()[1]);

    dp->simple_list.Clear();
    EXPECT_EQ(0U, dp->simple_list.GetValue().size());
}
