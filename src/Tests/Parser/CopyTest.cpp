#include "ParserTestBase.h"

class CopyTest : public ParserTestBase {};

TEST_F(CopyTest, CopyContentsFrom) {
    InitSimple();

    const std::string input = GetFullSimpleInput();

    Parser::ObjectPtr obj1 = ParseString(input);
    Parser::ObjectPtr obj2 = obj1->CloneTyped<Simple>(true, "TestObj2");

    EXPECT_NOT_NULL(obj2.get());
    EXPECT_EQ("Simple",  obj2->GetTypeName());
    EXPECT_EQ("TestObj2", obj2->GetName());
    SimplePtr sp = std::dynamic_pointer_cast<Simple>(obj2);
    EXPECT_NOT_NULL(sp.get());
    EXPECT_TRUE(sp->bool_val);
    EXPECT_EQ(-13,  sp->int_val);
    EXPECT_EQ(67U,  sp->uint_val);
    EXPECT_EQ(3.4f, sp->float_val);
    EXPECT_EQ("A quoted string", sp->str_val.GetValue());
    EXPECT_ENUM_EQ(SimpleEnum::kE2, sp->enum_val.GetValue());
    EXPECT_TRUE(sp->flag_val.GetValue().Has(FlagEnum::kF1));
    EXPECT_FALSE(sp->flag_val.GetValue().Has(FlagEnum::kF2));
    EXPECT_TRUE(sp->flag_val.GetValue().Has(FlagEnum::kF3));
    EXPECT_EQ(Vector3f(2.f, 3.f, 4.5f), sp->vec3f_val);
    EXPECT_EQ(Color(.2f, .3f, .4f, 1.f), sp->color_val);
    EXPECT_EQ(Anglef::FromDegrees(90), sp->angle_val);
    EXPECT_EQ(Rotationf::FromAxisAndAngle(Vector3f(0, 1, 0),
                                          Anglef::FromDegrees(180)),
              sp->rot_val);
    const std::vector<int> &ints = sp->ints_val.GetValue();
    EXPECT_EQ(3U, ints.size());
    EXPECT_EQ(6,  ints[0]);
    EXPECT_EQ(5,  ints[1]);
    EXPECT_EQ(-2, ints[2]);
    const std::vector<std::string> &strs = sp->strs_val.GetValue();
    EXPECT_EQ(2U,  strs.size());
    EXPECT_EQ("A", strs[0]);
    EXPECT_EQ("B", strs[1]);
}

TEST_F(CopyTest, CopyDerived) {
    InitDerived();

    auto sp0 = Parser::Registry::CreateObject<Simple>();
    auto sp1 = Parser::Registry::CreateObject<Simple>();
    auto sp2 = Parser::Registry::CreateObject<Simple>();

    sp0->int_val = 12;
    sp1->int_val = 13;
    sp2->int_val = 14;

    auto dp0 = Parser::Registry::CreateObject<Derived>();
    dp0->simple      = sp0;
    dp0->simple_list = std::vector<SimplePtr>{ sp1, sp2 };

    // Shallow clone.
    auto dp1 = dp0->CloneTyped<Derived>(false, "DP1");
    EXPECT_NOT_NULL(dp1.get());
    EXPECT_EQ("Derived",  dp1->GetTypeName());
    EXPECT_EQ("DP1",      dp1->GetName());
    EXPECT_EQ(sp0,        dp1->simple.GetValue());
    EXPECT_EQ(2U,         dp1->simple_list.GetValue().size());
    EXPECT_EQ(sp1,        dp1->simple_list.GetValue()[0]);
    EXPECT_EQ(sp2,        dp1->simple_list.GetValue()[1]);

    // Deep clone.
    auto dp2 = dp0->CloneTyped<Derived>(true, "DP2");
    EXPECT_NOT_NULL(dp2.get());
    EXPECT_EQ("Derived",  dp2->GetTypeName());
    EXPECT_EQ("DP2",      dp2->GetName());
    EXPECT_NE(sp0,        dp2->simple.GetValue());
    EXPECT_EQ(12,         dp2->simple.GetValue()->int_val.GetValue());
    EXPECT_EQ(2U,         dp2->simple_list.GetValue().size());
    EXPECT_NE(sp1,        dp2->simple_list.GetValue()[0]);
    EXPECT_NE(sp2,        dp2->simple_list.GetValue()[1]);
    EXPECT_EQ(13,         dp2->simple_list.GetValue()[0]->int_val.GetValue());
    EXPECT_EQ(14,         dp2->simple_list.GetValue()[1]->int_val.GetValue());
}
