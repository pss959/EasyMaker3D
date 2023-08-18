#include "Tests/Parser/ParserTestBase.h"
#include "Tests/TempFile.h"

/// \ingroup Tests
class ParsingTest : public ParserTestBase {
  protected:
    /// Tries parsing a value of a given type in a string and comparing with an
    /// expected field value.
    template <typename T>
    bool TryValue(Parser::TField<T> Simple::* field, T expected,
                  const Str &str){
        Parser::ObjectPtr obj = ParseString(str);
        if (! obj)
            return false;
        std::shared_ptr<Simple> sp = std::dynamic_pointer_cast<Simple>(obj);
        EXPECT_NOT_NULL(sp.get());
        return ((*sp).*field).GetValue() == expected;
    };
};

TEST_F(ParsingTest, StringAndFile) {
    const Str input = GetSimpleInput();

    // Set up a temporary file with the input string.
    TempFile tmp_file(input);

    // Parse both the string and the file and test the results.
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
        EXPECT_EQ(Vector3f(2, 3, 4.5f), sp->vec3f_val);
        EXPECT_EQ(Vector4f(2, 3, 4.5f, 5), sp->vec4f_val);
        EXPECT_EQ(Color(.2f, .3f, .4f, 1), sp->color_val);
        EXPECT_EQ(Anglef::FromDegrees(90), sp->angle_val);
        EXPECT_EQ(BuildRotation(0, 1, 0, 180), sp->rot_val);
        const std::vector<int> &ints = sp->ints_val.GetValue();
        EXPECT_EQ(3U, ints.size());
        EXPECT_EQ(6,  ints[0]);
        EXPECT_EQ(5,  ints[1]);
        EXPECT_EQ(-2, ints[2]);
        const StrVec &strs = sp->strs_val.GetValue();
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
        EXPECT_TRUE(sp->vec4f_val.WasSet());
        EXPECT_TRUE(sp->color_val.WasSet());
        EXPECT_TRUE(sp->angle_val.WasSet());
        EXPECT_TRUE(sp->rot_val.WasSet());
        EXPECT_TRUE(sp->ints_val.WasSet());
        EXPECT_TRUE(sp->strs_val.WasSet());
    }
}

TEST_F(ParsingTest, Derived) {
    const Str input = R"(
        # Full-line comment
        Derived "TestObj" {
          int_val:   13, # In-line comment
          float_val: 3.4,
          str_val:   "A quoted string",
          vec3f_val: 2 3 4.5,
          vec4f_val: "#aabbccdd",  # Scan as a color.
          vec4f_val: 2 3 4.5 5,
          simple: Simple "Nested" {
             int_val: 271,
          },
          simple_list: [
             Simple "Nested1" {},
             Simple "Nested2" {},
          ],
          hidden_int: 5,
        }"
)";

    Parser::ObjectPtr obj = ParseString(input);
    EXPECT_NOT_NULL(obj.get());
    EXPECT_EQ("Derived", obj->GetTypeName());
    EXPECT_EQ("TestObj", obj->GetName());
    DerivedPtr dp = std::dynamic_pointer_cast<Derived>(obj);
    EXPECT_NOT_NULL(dp.get());
    EXPECT_EQ(13,   dp->int_val);
    EXPECT_EQ(3.4f, dp->float_val);
    EXPECT_EQ("A quoted string", dp->str_val.GetValue());
    EXPECT_EQ(Vector3f(2, 3, 4.5f), dp->vec3f_val);
    EXPECT_EQ(Vector4f(2, 3, 4.5f, 5), dp->vec4f_val);

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

TEST_F(ParsingTest, Full) {
    // Test scanning/parsing every field type.
    Parser::ObjectPtr obj = ParseString(GetFullInput());
    EXPECT_NOT_NULL(obj.get());
    EXPECT_EQ("Full",     obj->GetTypeName());
    EXPECT_EQ("TestFull", obj->GetName());

    Util::Flags<FlagEnum> flags;
    flags.Set(FlagEnum::kF1);
    flags.Set(FlagEnum::kF3);
    Color c;
    EXPECT_TRUE(c.FromHexString("#aabbccdd"));
    const Anglef    a = Anglef::FromDegrees(-105);
    const Rotationf r = BuildRotation(Vector3f::AxisZ(), -80);
    const Matrix2f  m2(1, 2, 3, 4);
    const Matrix3f  m3(1, 2, 3, 4, 5, 6, 7, 8, 9);
    const Matrix4f  m4(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
    const Plane     pl(4.5f, Vector3f(0, 1, 0));
    const CircleArc ca(Anglef::FromDegrees(15), Anglef::FromDegrees(-250));

    FullPtr fp = std::dynamic_pointer_cast<Full>(obj);
    EXPECT_NOT_NULL(fp.get());
    EXPECT_EQ(true,    fp->b.GetValue());
    EXPECT_EQ(-6,      fp->i.GetValue());
    EXPECT_EQ(93U,     fp->u.GetValue());
    EXPECT_EQ(1234U,   fp->z.GetValue());
    EXPECT_EQ(3.5f,    fp->f.GetValue());
    EXPECT_EQ("A B C", fp->s.GetValue());
    EXPECT_EQ(flags,   fp->g.GetValue());
    EXPECT_ENUM_EQ(SimpleEnum::kE2, fp->e.GetValue());

    EXPECT_EQ(Vector2f(3, -2.5f),        fp->v2f.GetValue());
    EXPECT_EQ(Vector3f(3, -2.5f, 7),     fp->v3f.GetValue());
    EXPECT_EQ(Vector4f(3, -2.5f, 7, -2), fp->v4f.GetValue());
    EXPECT_EQ(Vector2i(3, -2),           fp->v2i.GetValue());
    EXPECT_EQ(Vector3i(3, -2, 8),        fp->v3i.GetValue());
    EXPECT_EQ(Vector4i(3, -2, 8, 9),     fp->v4i.GetValue());
    EXPECT_EQ(Vector2ui(3, 2),           fp->v2ui.GetValue());
    EXPECT_EQ(Vector3ui(3, 2, 8),        fp->v3ui.GetValue());
    EXPECT_EQ(Vector4ui(3, 2, 8, 9),     fp->v4ui.GetValue());
    EXPECT_EQ(Point2f(3, -2.5f),         fp->p2f.GetValue());
    EXPECT_EQ(Point3f(3, -2.5f, 7),      fp->p3f.GetValue());
    EXPECT_EQ(Point2i(3, -2),            fp->p2i.GetValue());
    EXPECT_EQ(c,                         fp->c.GetValue());
    EXPECT_EQ(a,                         fp->a.GetValue());
    EXPECT_EQ(r,                         fp->r.GetValue());
    EXPECT_EQ(m2,                        fp->m2.GetValue());
    EXPECT_EQ(m3,                        fp->m3.GetValue());
    EXPECT_EQ(m4,                        fp->m4.GetValue());
    EXPECT_EQ(pl,                        fp->pl.GetValue());
    EXPECT_EQ(ca,                        fp->ca.GetValue());
}

TEST_F(ParsingTest, BoolParsing) {
    auto try_func = [&](bool expected, const Str &str){
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
    auto try_func = [&](int expected, const Str &str){
        return TryValue(&Simple::int_val, expected, str); };

    EXPECT_TRUE(try_func(10,   "Simple { int_val: 10 }"));
    EXPECT_TRUE(try_func(-20,  "Simple { int_val: -20 }"));
    EXPECT_TRUE(try_func(30,   "Simple { int_val: +30 }"));
}

TEST_F(ParsingTest, UIntParsing) {
    auto try_func = [&](unsigned int expected, const Str &str){
        return TryValue(&Simple::uint_val, expected, str); };

    EXPECT_TRUE(try_func(10,   "Simple { uint_val: 10 }"));
    EXPECT_TRUE(try_func(62,   "Simple { uint_val: 076 }"));
    EXPECT_TRUE(try_func(2651, "Simple { uint_val: 0xa5b }"));
    EXPECT_TRUE(try_func(2651, "Simple { uint_val: 0XA5B }"));
}

TEST_F(ParsingTest, StringParsing) {
    auto try_func = [&](const Str &expected, const Str &str){
        return TryValue(&Simple::str_val, expected, str); };

    EXPECT_TRUE(try_func("Hello",   "Simple { str_val: \"Hello\" }"));
    EXPECT_TRUE(try_func("A \"Q\"", "Simple { str_val: \"A \\\"Q\\\"\" }"));

    // All special escaped characters.
    EXPECT_TRUE(
        try_func("A \a \b \f \n \r \t \v B",
                 "Simple { str_val: \"A \\a \\b \\f \\n \\r \\t \\v B\" }"));
}

TEST_F(ParsingTest, ColorParsing) {
    auto try_func = [&](const Color &expected, const Str &str){
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
