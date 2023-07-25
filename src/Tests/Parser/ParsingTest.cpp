#include "ParserTestBase.h"

class ParsingTest : public ParserTestBase {};

TEST_F(ParsingTest, StringAndFile) {
    const std::string input = GetFullSimpleInput();

    // Set up a temporary file with the input string.
    TempFile tmp_file(input);

    // Parse both the string and the file and test the results.
    InitSimple();
    Parser::ObjectPtr obj1 = ParseString(input);
    Parser::ObjectPtr obj2 = ParseFile(input);
    EXPECT_NOT_NULL(obj1);
    EXPECT_NOT_NULL(obj2);
    for (Parser::ObjectPtr obj: { obj1, obj2 }) {
        EXPECT_NOT_NULL(obj.get());
        EXPECT_EQ("Simple",  obj->GetTypeName());
        EXPECT_EQ("TestObj", obj->GetName());
        SimplePtr sp = std::dynamic_pointer_cast<Simple>(obj);
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

        // Make sure all fields are known to be set.
        EXPECT_TRUE(sp->bool_val.WasSet());
        EXPECT_TRUE(sp->int_val.WasSet());
        EXPECT_TRUE(sp->uint_val.WasSet());
        EXPECT_TRUE(sp->float_val.WasSet());
        EXPECT_TRUE(sp->str_val.WasSet());
        EXPECT_TRUE(sp->enum_val.WasSet());
        EXPECT_TRUE(sp->flag_val.WasSet());
        EXPECT_TRUE(sp->vec3f_val.WasSet());
        EXPECT_TRUE(sp->color_val.WasSet());
        EXPECT_TRUE(sp->angle_val.WasSet());
        EXPECT_TRUE(sp->rot_val.WasSet());
        EXPECT_TRUE(sp->ints_val.WasSet());
        EXPECT_TRUE(sp->strs_val.WasSet());
    }
}

TEST_F(ParsingTest, Derived) {
    const std::string input =
        "# Full-line comment\n"
        "Derived \"TestObj\" {\n"
        "  int_val:   13, # In-line comment\n"
        "  float_val: 3.4,\n"
        "  str_val:   \"A quoted string\",\n"
        "  vec3f_val: 2 3 4.5,\n"
        "  simple: Simple \"Nested\" {\n"
        "     int_val: 271,\n"
        "  },\n"
        "  simple_list: [\n"
        "     Simple \"Nested1\" {},\n"
        "     Simple \"Nested2\" {},\n"
        "  ],\n"
        "  hidden_int: 5,\n"
        "}\n";

    InitDerived();

    Parser::ObjectPtr obj = ParseString(input);
    EXPECT_NOT_NULL(obj.get());
    EXPECT_EQ("Derived", obj->GetTypeName());
    EXPECT_EQ("TestObj", obj->GetName());
    DerivedPtr dp = std::dynamic_pointer_cast<Derived>(obj);
    EXPECT_NOT_NULL(dp.get());
    EXPECT_EQ(13,   dp->int_val);
    EXPECT_EQ(3.4f, dp->float_val);
    EXPECT_EQ("A quoted string", dp->str_val.GetValue());
    EXPECT_EQ(Vector3f(2.f, 3.f, 4.5f), dp->vec3f_val);

    const SimplePtr simp = dp->simple;
    EXPECT_NOT_NULL(simp);
    EXPECT_EQ("Nested", simp->GetName());
    EXPECT_EQ(271, simp->int_val);

    const std::vector<SimplePtr> &list = dp->simple_list;
    EXPECT_EQ(2U, list.size());
    EXPECT_EQ("Nested1", list[0]->GetName());
    EXPECT_EQ("Nested2", list[1]->GetName());

    EXPECT_EQ(5, dp->hidden_int.GetValue());
    EXPECT_TRUE(dp->hidden_int.IsHidden());
}

TEST_F(ParsingTest, BoolParsing) {
    InitSimple();

    auto try_func = [&](bool expected, const std::string &str){
        return TryValue(&Simple::bool_val, expected, str); };

    EXPECT_TRUE(try_func(false, "Simple { bool_val: F }"));
    EXPECT_TRUE(try_func(false, "Simple { bool_val: f }"));
    EXPECT_TRUE(try_func(false, "Simple { bool_val: False }"));
    EXPECT_TRUE(try_func(false, "Simple { bool_val: fAlSe }"));

    EXPECT_TRUE(try_func(true,  "Simple { bool_val: T }"));
    EXPECT_TRUE(try_func(true,  "Simple { bool_val: t }"));
    EXPECT_TRUE(try_func(true,  "Simple { bool_val: TruE }"));
    EXPECT_TRUE(try_func(true,  "Simple { bool_val: True }"));
}

TEST_F(ParsingTest, IntParsing) {
    InitSimple();

    auto try_func = [&](int expected, const std::string &str){
        return TryValue(&Simple::int_val, expected, str); };

    EXPECT_TRUE(try_func(10,   "Simple { int_val: 10 }"));
    EXPECT_TRUE(try_func(-20,  "Simple { int_val: -20 }"));
    EXPECT_TRUE(try_func(30,   "Simple { int_val: +30 }"));
}

TEST_F(ParsingTest, UIntParsing) {
    InitSimple();

    auto try_func = [&](unsigned int expected, const std::string &str){
        return TryValue(&Simple::uint_val, expected, str); };

    EXPECT_TRUE(try_func(10,   "Simple { uint_val: 10 }"));
    EXPECT_TRUE(try_func(62,   "Simple { uint_val: 076 }"));
    EXPECT_TRUE(try_func(2651, "Simple { uint_val: 0xa5b }"));
    EXPECT_TRUE(try_func(2651, "Simple { uint_val: 0XA5B }"));
}

TEST_F(ParsingTest, StringParsing) {
    InitSimple();

    auto try_func = [&](const std::string &expected, const std::string &str){
        return TryValue(&Simple::str_val, expected, str); };

    EXPECT_TRUE(try_func("Hello",   "Simple { str_val: \"Hello\" }"));
    EXPECT_TRUE(try_func("A \"Q\"", "Simple { str_val: \"A \\\"Q\\\"\" }"));
}

TEST_F(ParsingTest, ColorParsing) {
    InitSimple();

    auto try_func = [&](const Color &expected, const std::string &str){
        return TryValue(&Simple::color_val, expected, str); };

    EXPECT_TRUE(try_func(Color(1, 1, 1, 1),
                         "Simple { color_val: \"#ffffffff\" }"));
    EXPECT_TRUE(try_func(Color(1, 1, 1, 1),
                         "Simple { color_val: \"#ffffff\" }"));
    EXPECT_TRUE(try_func(Color(1, 1, 1, 1),
                         "Simple { color_val: 1 1 1 1 }"));
    EXPECT_TRUE(try_func(Color(0, 1, .5f, 1),
                         "Simple { color_val: 0 1 .5 1 }"));
    EXPECT_TRUE(try_func(Color(1, 1, 1, 1),
                         "Simple { color_val: 255 255 255 255 }"));
    EXPECT_TRUE(try_func(Color(1, 0, 0, 1),
                         "Simple { color_val: 255 0 0 255 }"));
}
