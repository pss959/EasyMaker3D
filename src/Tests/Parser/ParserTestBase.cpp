#include "ParserTestBase.h"

#include "Parser/Registry.h"
#include "Util/Enum.h"

// ----------------------------------------------------------------------------
// Simple class.
// ----------------------------------------------------------------------------

Simple::~Simple() {
}

void Simple::AddFields() {
    AddField(bool_val.Init("bool_val", false));
    AddField(int_val.Init("int_val", 0));
    AddField(uint_val.Init("uint_val", 0));
    AddField(float_val.Init("float_val", 0));
    AddField(str_val.Init("str_val"));
    AddField(enum_val.Init("enum_val", SimpleEnum::kE1));
    AddField(flag_val.Init("flag_val"));
    AddField(vec3f_val.Init("vec3f_val"));
    AddField(color_val.Init("color_val"));
    AddField(angle_val.Init("angle_val"));
    AddField(rot_val.Init("rot_val"));
    AddField(ints_val.Init("ints_val"));
    AddField(strs_val.Init("strs_val"));
}

// ----------------------------------------------------------------------------
// Derived class.
// ----------------------------------------------------------------------------

Derived::Derived() {
    hidden_int.SetHidden(true);
}

void Derived::AddFields() {
    Simple::AddFields();
    AddField(simple.Init("simple"));
    AddField(simple_list.Init("simple_list"));
    AddField(hidden_int.Init("hidden_int", 12));
}

// ----------------------------------------------------------------------------
// ParserTestBase class.
// ----------------------------------------------------------------------------

void ParserTestBase::InitSimple() {
    Parser::Registry::AddType<Simple>("Simple");
}

void ParserTestBase::InitDerived() {
    InitSimple();
    Parser::Registry::AddType<Derived>("Derived");
}

std::string ParserTestBase::GetFullSimpleInput() {
    return
        "# Full-line comment\n"
        "Simple \"TestObj\" {\n"
        "  bool_val:  true,\n"
        "  int_val:   -13, # In-line comment\n"
        "  uint_val:  67,\n"
        "  float_val: 3.4,\n"
        "  str_val:   \"A quoted string\",\n"
        "  enum_val:  \"kE2\",\n"
        "  flag_val:  \"kF3| kF1\",\n"
        "  vec3f_val: 2 3 4.5,\n"
        "  color_val: .2 .3 .4 1,\n"
        "  angle_val: 90,\n"
        "  rot_val:   0 1 0 180,\n"
        "  ints_val:  [6, 5, -2],\n"
        "  strs_val:  [\"A\", \"B\"],\n"
        "}\n";
}

Parser::ObjectPtr ParserTestBase::ParseString(const std::string &input) {
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

Parser::ObjectPtr ParserTestBase::ParseFile(const std::string &input) {
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
