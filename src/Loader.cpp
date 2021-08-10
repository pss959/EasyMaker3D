#include "Loader.h"

#include "ion/gfx/shaderinputregistry.h"
#include <ion/gfx/shaderinputregistry.h>
#include <ion/gfxutils/shadersourcecomposer.h>
#include <ion/gfxutils/shapeutils.h>
#include <ion/math/angle.h>
#include <ion/math/matrix.h>
#include <ion/math/rotation.h>
#include <ion/math/vector.h>
#include <ion/port/fileutils.h>

#include "Parser/Parser.h"
#include "Transform.h"

using ion::gfx::NodePtr;
using ion::gfx::ShaderProgramPtr;
using ion::gfx::ShapePtr;
using ion::gfx::StateTable;
using ion::gfx::StateTablePtr;
using ion::gfxutils::PlanarShapeSpec;
using ion::gfxutils::ShaderManager;
using ion::gfxutils::StringComposer;
using ion::math::Anglef;
using ion::math::Matrix4f;
using ion::math::Rotationf;
using ion::math::Vector2f;
using ion::math::Vector3f;
using ion::math::Vector4f;

// ----------------------------------------------------------------------------
// Parser::ObjectSpec specifications for loaded types.
// ----------------------------------------------------------------------------

//! Shorthand macro.
#define FIELD_(NAME, COUNT, TYPE) { NAME, Parser::ValueType::TYPE, COUNT }

static const std::vector<Parser::ObjectSpec> node_specs_{
    { "Node",
      { FIELD_("enabled",          1, kBool),
        FIELD_("scale",            3, kFloat),
        FIELD_("rotation",         4, kFloat),
        FIELD_("translation",      3, kFloat),
        FIELD_("state_table",      1, kObject),
        FIELD_("shader",           1, kObject),
        FIELD_("shapes",           1, kObjectList),
        FIELD_("children",         1, kObjectList), }
    },
    { "StateTable",
      { FIELD_("clear_color",      4, kFloat),
        FIELD_("enable_cap",       1, kString),
        FIELD_("disable_cap",      1, kString), }
    },
    { "Shader",
      { FIELD_("vertex_program",   1, kString),
        FIELD_("geometry_program", 1, kString),
        FIELD_("fragment_program", 1, kString), }
    },
    { "Box",
      { FIELD_("size",             3, kFloat), }
    },
    { "Cylinder",
      { FIELD_("bottom_radius",    1, kFloat),
        FIELD_("top_radius",       1, kFloat),
        FIELD_("height",           1, kFloat),
        FIELD_("has_top_cap",      1, kBool),
        FIELD_("has_bottom_cap",   1, kBool),
        FIELD_("shaft_band_count", 1, kInteger),
        FIELD_("cap_band_count",   1, kInteger),
        FIELD_("sector_count",     1, kInteger), }
    },
    { "Ellipsoid",
      { FIELD_("longitude_start",  1, kFloat),
        FIELD_("longitude_end",    1, kFloat),
        FIELD_("latitude_start",   1, kFloat),
        FIELD_("latitude_end",     1, kFloat),
        FIELD_("band_count",       1, kInteger),
        FIELD_("sector_count",     1, kInteger),
        FIELD_("size",             3, kFloat), }
    },
    { "Polygon",
      { FIELD_("sides",            1, kInteger),
        FIELD_("plane_normal",     1, kString), }
    },
    { "Rectangle",
      { FIELD_("plane_normal",     1, kString),
        FIELD_("size",             2, kFloat), }
    },
};

#undef FIELD_

// ----------------------------------------------------------------------------
// Value type conversion helper functions.
// ----------------------------------------------------------------------------

static Anglef ToAnglef(const Parser::Field &field) {
    assert(field.spec.type  == Parser::ValueType::kFloat);
    assert(field.spec.count == 1);
    return Anglef::FromDegrees(field.GetValue<float>());
}

static Vector2f ToVector2f(const Parser::Field &field) {
    assert(field.spec.type  == Parser::ValueType::kFloat);
    assert(field.spec.count == 2);
    const std::vector<float> values = field.GetValues<float>();
    return Vector2f(values[0], values[1]);
}

static Vector3f ToVector3f(const Parser::Field &field) {
    assert(field.spec.type  == Parser::ValueType::kFloat);
    assert(field.spec.count == 3);
    const std::vector<float> values = field.GetValues<float>();
    return Vector3f(values[0], values[1], values[2]);
}

static Vector4f ToVector4f(const Parser::Field &field) {
    assert(field.spec.type  == Parser::ValueType::kFloat);
    assert(field.spec.count == 4);
    const std::vector<float> values = field.GetValues<float>();
    return Vector4f(values[0], values[1], values[2], values[3]);
}

static Rotationf ToRotationf(const Parser::Field &field) {
    const Vector4f vec = ToVector4f(field);
    return Rotationf::FromAxisAndAngle(Vector3f(vec[0], vec[1], vec[2]),
                                       Anglef::FromDegrees(vec[3]));
}

template <typename EnumType>
static bool ToEnum(const Parser::Field &field, EnumType &val) {
    assert(field.spec.type  == Parser::ValueType::kString);
    assert(field.spec.count == 1);
    return Util::EnumFromString<EnumType>(field.GetValue<std::string>(), val);
}

// ----------------------------------------------------------------------------
// Loader implementation.
// ----------------------------------------------------------------------------

Loader::Loader() : shader_manager_(new ion::gfxutils::ShaderManager()) {
}

Loader::~Loader() {
}

NodePtr Loader::LoadNodeResource(const std::string &path) {
    return LoadNode(FullPath("nodes", path));
}

NodePtr Loader::LoadNode(const std::string &path) {
    return ExtractNode_(*ParseFile_(path));
}

Parser::ObjectPtr Loader::ParseFile_(const std::string &path) {
    Parser::ObjectPtr root;
    try {
        Parser::Parser parser(node_specs_);
        root = parser.ParseFile(path);
    }
    catch (Parser::Exception &ex) {
        throw Exception(path, std::string("Parsing failed:\n") + ex.what());
    }
    return root;
}

NodePtr Loader::ExtractNode_(const Parser::Object &obj) {
    CheckObjectType_(obj, "Node");

    NodePtr node(new ion::gfx::Node);
    node->SetLabel(obj.name);

    const ion::gfx::ShaderInputRegistryPtr& global_reg =
        ion::gfx::ShaderInputRegistry::GetGlobalRegistry();

    // Use this to build a Matrix4f.
    Transform transform;

    for (const Parser::FieldPtr &field_ptr: obj.fields) {
        const Parser::Field &field = *field_ptr;
        if      (field.spec.name == "enabled")
            node->Enable(field.GetValue<bool>());
        else if (field.spec.name == "scale")
            transform.SetScale(ToVector3f(field));
        else if (field.spec.name == "rotation")
            transform.SetRotation(ToRotationf(field));
        else if (field.spec.name == "translation")
            transform.SetTranslation(ToVector3f(field));
        else if (field.spec.name == "state_table")
            node->SetStateTable(ExtractStateTable_(
                                    *field.GetValue<Parser::ObjectPtr>()));
        else if (field.spec.name == "shader")
            node->SetShaderProgram(ExtractShaderProgram_(
                                       *field.GetValue<Parser::ObjectPtr>()));
        else if (field.spec.name == "shapes")
            for (const Parser::ObjectPtr &shape_obj:
                     field.GetValue<std::vector<Parser::ObjectPtr>>())
                node->AddShape(ExtractShape_(*shape_obj));
        else if (field.spec.name == "children")
            for (const Parser::ObjectPtr &child_obj:
                     field.GetValue<std::vector<Parser::ObjectPtr>>())
                node->AddChild(ExtractNode_(*child_obj));
        else
            ThrowBadField_(obj, field);
    }
    if (transform.AnyComponentSet())
        node->AddUniform(
            global_reg->Create<ion::gfx::Uniform>("uModelviewMatrix",
                                                  transform.GetMatrix()));
    return node;
}

StateTablePtr Loader::ExtractStateTable_(const Parser::Object &obj) {
    CheckObjectType_(obj, "StateTable");

    StateTablePtr table(new StateTable);

    for (const Parser::FieldPtr &field_ptr: obj.fields) {
        const Parser::Field &field = *field_ptr;

        if (field.spec.name == "clear_color")
            table->SetClearColor(ToVector4f(field));
        else if (field.spec.name == "enable_cap" ||
                 field.spec.name == "disable_cap") {
            StateTable::Capability cap;
            if (! ToEnum<StateTable::Capability>(field, cap))
                ThrowEnumException_(obj, field, "StateTable::Capability");
            table->Enable(cap, field.spec.name == "enable_cap");
        }
        else
            ThrowBadField_(obj, field);
    }
    return table;
}

ShaderProgramPtr Loader::ExtractShaderProgram_(const Parser::Object &obj) {
    CheckObjectType_(obj, "Shader");

    ShaderManager::ShaderSourceComposerSet composer_set;

    // Create a temporary name if there is none.
    std::string name = obj.name;
    if (name.empty())
        name = "XXXX"; // Use NameManager somehow.

    // Lambda for StringComposer builder to make things simpler.
    auto scb = [this, obj, name](const char *suffix,
                                 const Parser::Field &field){
        const std::string path = FullPath("shaders",
                                          field.GetValue<std::string>());
        std::string source;
        if (! ion::port::ReadDataFromFile(path, &source))
            throw Exception(obj.path, obj.line_number,
                            "Unabled to read contents of '" + path +
                            "' for field '" + field.spec.name + "'");
        return new StringComposer(name + suffix, source);
    };

    for (const Parser::FieldPtr &field_ptr: obj.fields) {
        const Parser::Field &field = *field_ptr;
        if (field.spec.name == "vertex_program")
            composer_set.vertex_source_composer   = scb("_vp", field);
        else if (field.spec.name == "geometry_program")
            composer_set.geometry_source_composer = scb("_gp", field);
        else if (field.spec.name == "fragment_program")
            composer_set.fragment_source_composer = scb("_fp", field);
        else
            ThrowBadField_(obj, field);
    }

    // XXXX Need to allow uniforms to be registered.
    ion::gfx::ShaderInputRegistryPtr reg(new ion::gfx::ShaderInputRegistry);
    reg->IncludeGlobalRegistry();

    ShaderProgramPtr program =
        shader_manager_->CreateShaderProgram(name, reg, composer_set);
    if (! program->GetInfoLog().empty())
        throw Exception(obj.path, obj.line_number,
                        "Unable to compile shader program: " +
                        program->GetInfoLog());
    return program;
}

ShapePtr Loader::ExtractShape_(const Parser::Object &obj) {
    ShapePtr shape;
    if (obj.spec.type_name == "Box")
        shape = ExtractBox_(obj);
    else if (obj.spec.type_name == "Cylinder")
        shape = ExtractCylinder_(obj);
    else if (obj.spec.type_name == "Ellipsoid")
        shape = ExtractEllipsoid_(obj);
    else if (obj.spec.type_name == "Polygon")
        shape = ExtractPolygon_(obj);
    else if (obj.spec.type_name == "Rectangle")
        shape = ExtractRectangle_(obj);
    else
        ThrowTypeMismatch_(obj, "Some type of shape");
    shape->SetLabel(obj.name);
    return shape;
}

ShapePtr Loader::ExtractBox_(const Parser::Object &obj) {
    ion::gfxutils::BoxSpec spec;
    spec.vertex_type = ion::gfxutils::ShapeSpec::kPosition;
    for (const Parser::FieldPtr &field_ptr: obj.fields) {
        const Parser::Field &field = *field_ptr;
        if (field.spec.name == "size")
            spec.size = ToVector3f(field);
        else
            ThrowBadField_(obj, field);
    }
    ShapePtr shape = ion::gfxutils::BuildBoxShape(spec);
    return shape;
}

ShapePtr Loader::ExtractCylinder_(const Parser::Object &obj) {
    ion::gfxutils::CylinderSpec spec;
    spec.vertex_type = ion::gfxutils::ShapeSpec::kPosition;
    for (const Parser::FieldPtr &field_ptr: obj.fields) {
        const Parser::Field &field = *field_ptr;
        if      (field.spec.name == "bottom_radius")
            spec.bottom_radius = field.GetValue<float>();
        else if (field.spec.name == "top_radius")
            spec.top_radius = field.GetValue<float>();
        else if (field.spec.name == "height")
            spec.height = field.GetValue<float>();
        else if (field.spec.name == "has_top_cap")
            spec.has_top_cap = field.GetValue<bool>();
        else if (field.spec.name == "has_bottom_cap")
            spec.has_bottom_cap = field.GetValue<bool>();
        else if (field.spec.name == "shaft_band_count")
            spec.shaft_band_count = field.GetValue<int>();
        else if (field.spec.name == "cap_band_count")
            spec.cap_band_count = field.GetValue<int>();
        else if (field.spec.name == "sector_count")
            spec.sector_count = field.GetValue<int>();
        else
            ThrowBadField_(obj, field);
    }
    ShapePtr shape = ion::gfxutils::BuildCylinderShape(spec);
    return shape;
}

ShapePtr Loader::ExtractEllipsoid_(const Parser::Object &obj) {
    ion::gfxutils::EllipsoidSpec spec;
    spec.vertex_type = ion::gfxutils::ShapeSpec::kPosition;
    for (const Parser::FieldPtr &field_ptr: obj.fields) {
        const Parser::Field &field = *field_ptr;
        if      (field.spec.name == "longitude_start")
            spec.longitude_start = ToAnglef(field);
        else if (field.spec.name == "longitude_end")
            spec.longitude_end = ToAnglef(field);
        else if (field.spec.name == "latitude_start")
            spec.latitude_start = ToAnglef(field);
        else if (field.spec.name == "latitude_end")
            spec.latitude_end = ToAnglef(field);
        else if (field.spec.name == "band_count")
            spec.band_count = field.GetValue<int>();
        else if (field.spec.name == "sector_count")
            spec.sector_count = field.GetValue<int>();
        else if (field.spec.name == "size")
            spec.size = ToVector3f(field);
        else
            ThrowBadField_(obj, field);
    }
    ShapePtr shape = ion::gfxutils::BuildEllipsoidShape(spec);
    return shape;
}

ShapePtr Loader::ExtractPolygon_(const Parser::Object &obj) {
    ion::gfxutils::RegularPolygonSpec spec;
    spec.vertex_type = ion::gfxutils::ShapeSpec::kPosition;
    for (const Parser::FieldPtr &field_ptr: obj.fields) {
        const Parser::Field &field = *field_ptr;
        if      (field.spec.name == "sides")
            spec.sides = field.GetValue<int>();
        else if (field.spec.name == "plane_normal") {
            if (! ToEnum<PlanarShapeSpec::PlaneNormal>(field,
                                                       spec.plane_normal))
                ThrowEnumException_(obj, field, "PlanarShapeSpec::PlaneNormal");
        }
        else
            ThrowBadField_(obj, field);
    }
    ShapePtr shape = ion::gfxutils::BuildRegularPolygonShape(spec);
    return shape;
}

ShapePtr Loader::ExtractRectangle_(const Parser::Object &obj) {
    ion::gfxutils::RectangleSpec spec;
    spec.vertex_type = ion::gfxutils::ShapeSpec::kPosition;
    for (const Parser::FieldPtr &field_ptr: obj.fields) {
        const Parser::Field &field = *field_ptr;
        if      (field.spec.name == "plane_normal") {
            if (! ToEnum<PlanarShapeSpec::PlaneNormal>(field,
                                                       spec.plane_normal))
                ThrowEnumException_(obj, field, "PlanarShapeSpec::PlaneNormal");
        }
        else if (field.spec.name == "size")
            spec.size = ToVector2f(field);
        else
            ThrowBadField_(obj, field);
    }
    ShapePtr shape = ion::gfxutils::BuildRectangleShape(spec);
    return shape;
}

void Loader::CheckObjectType_(const Parser::Object &obj,
                              const std::string &expected_type) {
    if (obj.spec.type_name != expected_type)
        ThrowTypeMismatch_(obj, expected_type);
}

void Loader::ThrowTypeMismatch_(const Parser::Object &obj,
                                const std::string &expected_type) {
    throw Exception(obj.path, obj.line_number,
                    "Expected " + expected_type +
                    " object, got " + obj.spec.type_name);
}

void Loader::ThrowMissingField_(const Parser::Object &obj,
                                const std::string &field_name) {
    throw Exception(obj.path, obj.line_number,
                    "Missing field '" + field_name +
                    "' in " + obj.spec.type_name + " object");
}

void Loader::ThrowBadField_(const Parser::Object &obj,
                            const Parser::Field &field) {
    throw Exception(obj.path, obj.line_number,
                    "Invalid field '" + field.spec.name +
                    "' found in " + obj.spec.type_name + " object");
}

void Loader::ThrowEnumException_(const Parser::Object &obj,
                                 const Parser::Field &field,
                                 const std::string &enum_type_name) {
    throw Exception(obj.path, obj.line_number,
                    "Invalid value string '" +
                    field.GetValue<std::string>() + "' for enum of type " +
                    enum_type_name);
}
