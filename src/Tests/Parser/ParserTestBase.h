//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include "Math/Color.h"
#include "Math/Curves.h"
#include "Math/Plane.h"
#include "Math/Types.h"
#include "Parser/Exception.h"
#include "Parser/Field.h"
#include "Parser/Object.h"
#include "Parser/Parser.h"
#include "Tests/SceneTestBase.h"
#include "Tests/Testing2.h"
#include "Util/Memory.h"

// Tests that a Parser::Exception is thrown and that its message contains
// the given string pattern.
#define TEST_THROW_(STMT, PATTERN) \
    TEST_THROW(STMT, Parser::Exception, PATTERN)

// ----------------------------------------------------------------------------
// Parser test base class.
// ----------------------------------------------------------------------------

/// Base class that sets up a Parser.
/// \ingroup Tests
class ParserTestBase : public SceneTestBase {
 protected:
    enum class SimpleEnum { kE1, kE2, kE3 };
    enum class FlagEnum   { kF1 = 0x1, kF2 = 0x2, kF3 = 0x4 };

    // These nested classes are all defined below.
    class Simple;
    class Derived;
    class Full;
    class Other;
    class Unscoped;

    // Shorthand.
    DECL_SHARED_PTR(Simple);
    DECL_SHARED_PTR(Derived);
    DECL_SHARED_PTR(Full);
    DECL_SHARED_PTR(Other);

    Parser::Parser parser;

    // The constructor calls InitTestClasses() by default.
    ParserTestBase();

    // Sets up the Parser to use all of the classes defined above.
    void InitTestClasses();

    // Returns a string used to initialize all fields in a Simple instance.
    static Str GetSimpleInput();

    // Returns a string used to initialize all fields in a Full instance.
    static Str GetFullInput();

    // Parses the given string, checking for exceptions. Returns a null
    // ObjectPtr on failure.
    Parser::ObjectPtr ParseString(const Str &input);

    // Sets up a temporary file containing the given input string, parses it,
    // and returns the result. Returns a null ObjectPtr on failure.
    Parser::ObjectPtr ParseFile(const Str &input);
};

// ----------------------------------------------------------------------------
// Nested classes for testing parsing.
// ----------------------------------------------------------------------------

/// Class with all non-object parser value types.
class ParserTestBase::Simple : public Parser::Object {
  public:
    virtual void AddFields() override;

    Parser::TField<bool>          bool_val;
    Parser::TField<int>            int_val;
    Parser::TField<unsigned int>  uint_val;
    Parser::TField<float>        float_val;
    Parser::TField<Str>            str_val;
    Parser::EnumField<SimpleEnum> enum_val;
    Parser::FlagField<FlagEnum>   flag_val;
    Parser::TField<Vector3f>     vec3f_val;
    Parser::TField<Vector4f>     vec4f_val;
    Parser::TField<Color>        color_val;
    Parser::TField<Anglef>       angle_val;
    Parser::TField<Rotationf>      rot_val;
    Parser::VField<int>           ints_val;
    Parser::VField<Str>           strs_val;
  protected:
    virtual ~Simple();
    friend class Parser::Registry;
};

/// Derived class that adds Object and ObjectList value types.
class ParserTestBase::Derived : public Simple {
  public:
    // Makes this publicly available.
    bool IsCreationDone2() const { return IsCreationDone(); }

    virtual void AddFields() override;

    Parser::ObjectField<Simple>     simple;
    Parser::ObjectListField<Simple> simple_list;
    Parser::TField<int>             hidden_int;
  protected:
    Derived();
    friend class Parser::Registry;
};

/// Class with a field for each supported type.
class ParserTestBase::Full : public Parser::Object {
  public:
    virtual void AddFields() override;

    Parser::TField<bool>          b;
    Parser::TField<int>           i;
    Parser::TField<unsigned int>  u;
    Parser::TField<size_t>        z;
    Parser::TField<float>         f;
    Parser::TField<Str>           s;
    Parser::EnumField<SimpleEnum> e;
    Parser::FlagField<FlagEnum>   g;
    Parser::TField<Vector2f>      v2f;
    Parser::TField<Vector3f>      v3f;
    Parser::TField<Vector4f>      v4f;
    Parser::TField<Vector2i>      v2i;
    Parser::TField<Vector3i>      v3i;
    Parser::TField<Vector4i>      v4i;
    Parser::TField<Vector2ui>     v2ui;
    Parser::TField<Vector3ui>     v3ui;
    Parser::TField<Vector4ui>     v4ui;
    Parser::TField<Point2f>       p2f;
    Parser::TField<Point3f>       p3f;
    Parser::TField<Point2ui>      p2ui;
    Parser::TField<Color>         c;
    Parser::TField<Anglef>        a;
    Parser::TField<Rotationf>     r;
    Parser::TField<Matrix2f>      m2;
    Parser::TField<Matrix3f>      m3;
    Parser::TField<Matrix4f>      m4;
    Parser::TField<Plane>         pl;
    Parser::TField<CircleArc>     ca;

  protected:
    virtual ~Full();
    friend class Parser::Registry;
};

/// Class used for testing type and other errors.
class ParserTestBase::Other : public Parser::Object {
  protected:
    Other() {}
    friend class Parser::Registry;
};

/// Class used to test overriding IsScoped() and unnamed object errors.
class ParserTestBase::Unscoped : public Parser::Object {
  public:
    virtual bool IsScoped() const override { return false; }
    virtual bool IsNameRequired() const override { return true; }
  protected:
    Unscoped() {}
    virtual bool IsValid(Str &details) override;
    friend class Parser::Registry;
};
