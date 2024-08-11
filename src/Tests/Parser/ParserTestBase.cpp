//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Tests/Parser/ParserTestBase.h"

#include "Parser/Registry.h"
#include "Tests/TempFile.h"
#include "Util/Enum.h"

// ----------------------------------------------------------------------------
// Test class functions.
// ----------------------------------------------------------------------------

ParserTestBase::Simple::~Simple() {
}

void ParserTestBase::Simple::AddFields() {
    AddField(bool_val.Init("bool_val", false));
    AddField(int_val.Init("int_val", 0));
    AddField(uint_val.Init("uint_val", 0));
    AddField(float_val.Init("float_val", 0));
    AddField(str_val.Init("str_val"));
    AddField(enum_val.Init("enum_val", SimpleEnum::kE1));
    AddField(flag_val.Init("flag_val"));
    AddField(vec3f_val.Init("vec3f_val"));
    AddField(vec4f_val.Init("vec4f_val"));
    AddField(color_val.Init("color_val"));
    AddField(angle_val.Init("angle_val"));
    AddField(rot_val.Init("rot_val"));
    AddField(ints_val.Init("ints_val"));
    AddField(strs_val.Init("strs_val"));
}

ParserTestBase::Derived::Derived() {
    hidden_int.SetHidden(true);
}

void ParserTestBase::Derived::AddFields() {
    Simple::AddFields();
    AddField(simple.Init("simple"));
    AddField(simple_list.Init("simple_list"));
    AddField(hidden_int.Init("hidden_int", 12));
}

ParserTestBase::Full::~Full() {
}

void ParserTestBase::Full::AddFields() {
    AddField(b.Init("b",       false));
    AddField(i.Init("i",       0));
    AddField(u.Init("u",       0));
    AddField(z.Init("z",       0));
    AddField(f.Init("f",       0));
    AddField(s.Init("s"));
    AddField(e.Init("e",       SimpleEnum::kE1));
    AddField(g.Init("g"));
    AddField(v2f.Init("v2f",   Vector2f::Zero()));
    AddField(v3f.Init("v3f",   Vector3f::Zero()));
    AddField(v4f.Init("v4f",   Vector4f::Zero()));
    AddField(v2i.Init("v2i",   Vector2i::Zero()));
    AddField(v3i.Init("v3i",   Vector3i::Zero()));
    AddField(v4i.Init("v4i",   Vector4i::Zero()));
    AddField(v2ui.Init("v2ui", Vector2ui::Zero()));
    AddField(v3ui.Init("v3ui", Vector3ui::Zero()));
    AddField(v4ui.Init("v4ui", Vector4ui::Zero()));
    AddField(p2f.Init("p2f",   Point2f::Zero()));
    AddField(p3f.Init("p3f",   Point3f::Zero()));
    AddField(p2ui.Init("p2ui", Point2ui::Zero()));
    AddField(c.Init("c"));
    AddField(a.Init("a"));
    AddField(r.Init("r"));
    AddField(m2.Init("m2"));
    AddField(m3.Init("m3"));
    AddField(m4.Init("m4"));
    AddField(pl.Init("pl"));
    AddField(ca.Init("ca"));
}

bool ParserTestBase::Unscoped::IsValid(Str &details) {
    if (GetName() == "INVALID") {
        details = "invalid name";
        return false;
    }
    return true;
}

// ----------------------------------------------------------------------------
// ParserTestBase class.
// ----------------------------------------------------------------------------

ParserTestBase::ParserTestBase() {
    InitTestClasses();
}

void ParserTestBase::InitTestClasses() {
    Parser::Registry::AddType<Simple>("Simple");
    Parser::Registry::AddType<Derived>("Derived");
    Parser::Registry::AddType<Full>("Full");
    Parser::Registry::AddType<Other>("Other");
    Parser::Registry::AddType<Unscoped>("Unscoped");
}

Str ParserTestBase::GetSimpleInput() {
    return R"(
        # Full-line comment
        Simple "TestObj" {
          bool_val:  true,
          int_val:   -13, # In-line comment
          uint_val:  67,
          float_val: 3.4,
          str_val:   "A quoted string",
          enum_val:  "kE2",
          flag_val:  "kF3| kF1",
          vec3f_val: 2 3 4.5,
          vec4f_val: 2 3 4.5 5,
          color_val: .2 .3 .4 1,
          angle_val: 90,
          rot_val:   0 1 0 180,
          ints_val:  [6, 5],
          ints_val:  [-2],  # Accumulates.
          strs_val:  ["A", "B"],
        })";
}

Str ParserTestBase::GetFullInput() {
    return R"(
        Full "TestFull" {
          b: True,
          i: -6,
          u: 93,
          z: 1234,
          f: 3.5,
          s: "A B C",
          e: "kE2",
          g: "kF3|kF1",
          v2f: 3 -2.5,
          v3f: 3 -2.5 7,
          v4f: 3 -2.5 7 -2,
          v2i: 3 -2,
          v3i: 3 -2 8,
          v4i: 3 -2 8 9,
          v2ui: 3 2,
          v3ui: 3 2 8,
          v4ui: 3 2 8 9,
          p2f: 3 -2.5,
          p3f: 3 -2.5 7,
          p2ui: 3 2,
          c: "#aabbccdd",
          a: -105,
          r: 0 0 1 -80,
          m2: 1 2 3 4,
          m3: 1 2 3 4 5 6 7 8 9,
          m4: 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16,
          pl: 0 1 0 4.5,
          ca: 15 -250,
        })";
}

Parser::ObjectPtr ParserTestBase::ParseString(const Str &input) {
    Parser::ObjectPtr obj;
    try {
        obj = parser.ParseFromString(input);
    }
    catch (const Parser::Exception &ex) {
        std::cerr << "*** EXCEPTION; " << ex.what() << "\n";
        obj.reset();
    }
    return obj;
}

Parser::ObjectPtr ParserTestBase::ParseFile(const Str &input) {
    TempFile tmp_file(input);
    Parser::ObjectPtr obj;
    try {
        obj = parser.ParseFile(tmp_file.GetPath());
    }
    catch (const Parser::Exception &ex) {
        std::cerr << "*** EXCEPTION; " << ex.what() << "\n";
        obj.reset();
    }
    return obj;
}
