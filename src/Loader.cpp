#include "Loader.h"

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
using ion::gfx::ShaderInputRegistry;
using ion::gfx::ShaderInputRegistryPtr;
using ion::gfx::ShaderProgramPtr;
using ion::gfx::ShapePtr;
using ion::gfx::StateTable;
using ion::gfx::StateTablePtr;
using ion::gfx::Uniform;
using ion::gfxutils::PlanarShapeSpec;
using ion::gfxutils::ShaderManager;
using ion::gfxutils::StringComposer;
using ion::math::Anglef;
using ion::math::Matrix2f;
using ion::math::Matrix3f;
using ion::math::Matrix4f;
using ion::math::Rotationf;
using ion::math::Vector2f;
using ion::math::Vector3f;
using ion::math::Vector4f;
using ion::math::Vector2i;
using ion::math::Vector3i;
using ion::math::Vector4i;
using ion::math::Vector2ui;
using ion::math::Vector3ui;
using ion::math::Vector4ui;

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
        FIELD_("uniforms",         1, kObjectList),
        FIELD_("shapes",           1, kObjectList),
        FIELD_("children",         1, kObjectList), }
    },
    { "StateTable",
      { FIELD_("clear_color",      4, kFloat),
        FIELD_("enable_cap",       1, kString),
        FIELD_("disable_cap",      1, kString), }
    },
    { "Shader",
      { FIELD_("uniform_defs",     1, kObjectList),
        FIELD_("vertex_program",   1, kString),
        FIELD_("geometry_program", 1, kString),
        FIELD_("fragment_program", 1, kString), }
    },
    { "UniformDef",
      { FIELD_("type",             1, kString), }
    },
    { "Uniform",
      { FIELD_("float_val",        1, kFloat),
        FIELD_("int_val",          1, kInteger),
        FIELD_("uint_val",         1, kUInteger),
        FIELD_("vec2f_val",        2, kFloat),
        FIELD_("vec3f_val",        3, kFloat),
        FIELD_("vec4f_val",        4, kFloat),
        FIELD_("vec2i_val",        2, kInteger),
        FIELD_("vec3i_val",        3, kInteger),
        FIELD_("vec4i_val",        4, kInteger),
        FIELD_("vec2ui_val",       2, kUInteger),
        FIELD_("vec3ui_val",       3, kUInteger),
        FIELD_("vec4ui_val",       4, kUInteger),
        FIELD_("mat2_val",         4, kFloat),
        FIELD_("mat3_val",         9, kFloat),
        FIELD_("mat4_val",        16, kFloat),
      }
    },

    // Shapes:
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

static void Validate_(const Parser::Field &field,
                      uint32_t count, Parser::ValueType type) {
    assert(field.spec.type  == type);
    assert(field.spec.count == count);
}

static Anglef ToAnglef_(const Parser::Field &field) {
    Validate_(field, 1, Parser::ValueType::kFloat);
    return Anglef::FromDegrees(field.GetValue<float>());
}

static Vector2f ToVector2f_(const Parser::Field &field) {
    Validate_(field, 2, Parser::ValueType::kFloat);
    const std::vector<float> values = field.GetValues<float>();
    return Vector2f(values[0], values[1]);
}

static Vector3f ToVector3f_(const Parser::Field &field) {
    Validate_(field, 3, Parser::ValueType::kFloat);
    const std::vector<float> values = field.GetValues<float>();
    return Vector3f(values[0], values[1], values[2]);
}

static Vector4f ToVector4f_(const Parser::Field &field) {
    Validate_(field, 4, Parser::ValueType::kFloat);
    const std::vector<float> values = field.GetValues<float>();
    return Vector4f(values[0], values[1], values[2], values[3]);
}

static Vector2i ToVector2i_(const Parser::Field &field) {
    Validate_(field, 2, Parser::ValueType::kInteger);
    const std::vector<int> values = field.GetValues<int>();
    return Vector2i(values[0], values[1]);
}

static Vector3i ToVector3i_(const Parser::Field &field) {
    Validate_(field, 3, Parser::ValueType::kInteger);
    const std::vector<int> values = field.GetValues<int>();
    return Vector3i(values[0], values[1], values[2]);
}

static Vector4i ToVector4i_(const Parser::Field &field) {
    Validate_(field, 4, Parser::ValueType::kInteger);
    const std::vector<int> values = field.GetValues<int>();
    return Vector4i(values[0], values[1], values[2], values[3]);
}

static Vector2i ToVector2ui_(const Parser::Field &field) {
    Validate_(field, 2, Parser::ValueType::kUInteger);
    const std::vector<int> values = field.GetValues<int>();
    return Vector2i(values[0], values[1]);
}

static Vector3i ToVector3ui_(const Parser::Field &field) {
    Validate_(field, 3, Parser::ValueType::kUInteger);
    const std::vector<int> values = field.GetValues<int>();
    return Vector3i(values[0], values[1], values[2]);
}

static Vector4i ToVector4ui_(const Parser::Field &field) {
    Validate_(field, 4, Parser::ValueType::kUInteger);
    const std::vector<int> values = field.GetValues<int>();
    return Vector4i(values[0], values[1], values[2], values[3]);
}

static Rotationf ToRotationf_(const Parser::Field &field) {
    const Vector4f vec = ToVector4f_(field);
    return Rotationf::FromAxisAndAngle(Vector3f(vec[0], vec[1], vec[2]),
                                       Anglef::FromDegrees(vec[3]));
}

static Matrix2f ToMatrix2f_(const Parser::Field &field) {
    Validate_(field, 4, Parser::ValueType::kFloat);
    const std::vector<float> values = field.GetValues<float>();
    return Matrix2f(values[0], values[1],
                    values[2], values[3]);
}

static Matrix3f ToMatrix3f_(const Parser::Field &field) {
    Validate_(field, 9, Parser::ValueType::kFloat);
    const std::vector<float> values = field.GetValues<float>();
    return Matrix3f(values[0], values[1], values[2],
                    values[3], values[4], values[5],
                    values[6], values[7], values[8]);
}

static Matrix4f ToMatrix4f_(const Parser::Field &field) {
    Validate_(field, 16, Parser::ValueType::kFloat);
    const std::vector<float> values = field.GetValues<float>();
    return Matrix4f(values[0], values[1], values[2], values[3],
                    values[4], values[5], values[6], values[7],
                    values[8], values[9], values[10], values[11],
                    values[12], values[13], values[14], values[15]);
}

template <typename EnumType>
static bool ToEnum_(const Parser::Field &field, EnumType &val) {
    Validate_(field, 1, Parser::ValueType::kString);
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
    return ExtractNode_(*ParseFile_(path), ShaderProgramPtr());
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

NodePtr Loader::ExtractNode_(const Parser::Object &obj,
                             const ShaderProgramPtr &cur_shader) {
    CheckObjectType_(obj, "Node");

    NodePtr node(new ion::gfx::Node);
    node->SetLabel(obj.name);

    // Use this to build a Matrix4f.
    Transform transform;

    // Saves Uniform objects to add after all fields are parsed, since the
    // shader must be known.
    std::vector<Parser::ObjectPtr> uniform_objs;

    // Saves child Node objects to add after all fields are parsed, since the
    // shader must be known.
    std::vector<Parser::ObjectPtr> child_objs;

    for (const Parser::FieldPtr &field_ptr: obj.fields) {
        const Parser::Field &field = *field_ptr;
        if      (field.spec.name == "enabled")
            node->Enable(field.GetValue<bool>());
        else if (field.spec.name == "scale")
            transform.SetScale(ToVector3f_(field));
        else if (field.spec.name == "rotation")
            transform.SetRotation(ToRotationf_(field));
        else if (field.spec.name == "translation")
            transform.SetTranslation(ToVector3f_(field));
        else if (field.spec.name == "state_table")
            node->SetStateTable(ExtractStateTable_(
                                    *field.GetValue<Parser::ObjectPtr>()));
        else if (field.spec.name == "shader")
            node->SetShaderProgram(ExtractShaderProgram_(
                                       *field.GetValue<Parser::ObjectPtr>()));
        else if (field.spec.name == "uniforms")
            uniform_objs = field.GetValue<std::vector<Parser::ObjectPtr>>();
        else if (field.spec.name == "shapes")
            for (const Parser::ObjectPtr &shape_obj:
                     field.GetValue<std::vector<Parser::ObjectPtr>>())
                node->AddShape(ExtractShape_(*shape_obj));
        else if (field.spec.name == "children")
            child_objs = field.GetValue<std::vector<Parser::ObjectPtr>>();
        else
            ThrowBadField_(obj, field);
    }

    // Determine the ShaderInputRegistry to use for uniforms. If the node has a
    // ShaderProgram, use its registry. Otherwise, use the one passed from
    // above, if it is not null. Otherwise, use the global registry.
    ShaderProgramPtr shader_program = node->GetShaderProgram();
    if (! shader_program)
        shader_program = cur_shader;
    ShaderInputRegistryPtr reg = shader_program ?
        shader_program->GetRegistry() :
        ShaderInputRegistry::GetGlobalRegistry();

    // Add a matrix uniform if there was any transform.
    if (transform.AnyComponentSet())
        node->AddUniform(reg->Create<Uniform>("uModelviewMatrix",
                                              transform.GetMatrix()));

    // Add any other uniforms.
    for (const Parser::ObjectPtr &uniform_obj: uniform_objs)
        node->AddUniform(ExtractUniform_(*uniform_obj, *reg));

    // Process children.
    for (const Parser::ObjectPtr &child_obj: child_objs)
        node->AddChild(ExtractNode_(*child_obj, shader_program));

    return node;
}

StateTablePtr Loader::ExtractStateTable_(const Parser::Object &obj) {
    CheckObjectType_(obj, "StateTable");

    StateTablePtr table(new StateTable);

    for (const Parser::FieldPtr &field_ptr: obj.fields) {
        const Parser::Field &field = *field_ptr;

        if (field.spec.name == "clear_color")
            table->SetClearColor(ToVector4f_(field));
        else if (field.spec.name == "enable_cap" ||
                 field.spec.name == "disable_cap") {
            StateTable::Capability cap;
            if (! ToEnum_<StateTable::Capability>(field, cap))
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

    // There has to be a name for the shader.
    if (obj.name.empty())
        ThrowMissingName_(obj);

    // Lambda for StringComposer builder to make things simpler.
    auto scb = [this, obj](const char *suffix,
                           const Parser::Field &field){
        const std::string path = FullPath("shaders",
                                          field.GetValue<std::string>());
        std::string source;
        if (! ion::port::ReadDataFromFile(path, &source))
            throw Exception(obj, "Unabled to read contents of '" + path +
                            "' for field '" + field.spec.name + "'");
        return new StringComposer(obj.name + suffix, source);
    };

    // Create a registry in case there are uniforms to be registered.
    ShaderInputRegistryPtr reg(new ShaderInputRegistry);
    reg->IncludeGlobalRegistry();

    for (const Parser::FieldPtr &field_ptr: obj.fields) {
        const Parser::Field &field = *field_ptr;
        if      (field.spec.name == "uniform_defs") {
            for (const Parser::ObjectPtr &uniform_obj:
                     field.GetValue<std::vector<Parser::ObjectPtr>>())
                ExtractAndAddUniformDef_(*uniform_obj, *reg);
        }
        else if (field.spec.name == "vertex_program")
            composer_set.vertex_source_composer   = scb("_vp", field);
        else if (field.spec.name == "geometry_program")
            composer_set.geometry_source_composer = scb("_gp", field);
        else if (field.spec.name == "fragment_program")
            composer_set.fragment_source_composer = scb("_fp", field);
        else
            ThrowBadField_(obj, field);
    }

    ShaderProgramPtr program =
        shader_manager_->CreateShaderProgram(obj.name, reg, composer_set);
    if (! program->GetInfoLog().empty())
        throw Exception(obj, "Unable to compile shader program: " +
                        program->GetInfoLog());
    return program;
}

void Loader::ExtractAndAddUniformDef_(const Parser::Object &obj,
                                      ShaderInputRegistry &reg) {
    // There has to be a name for the uniform.
    if (obj.name.empty())
        ThrowMissingName_(obj);

    Uniform::ValueType type;
    for (const Parser::FieldPtr &field_ptr: obj.fields) {
        const Parser::Field &field = *field_ptr;
        if (field.spec.name == "type") {
            if (! ToEnum_<Uniform::ValueType>(field, type))
                ThrowEnumException_(obj, field, "Uniform::ValueType");
        }
        else
            ThrowBadField_(obj, field);
    }

    ShaderInputRegistry::UniformSpec spec(obj.name, type);
    reg.Add<Uniform>(ShaderInputRegistry::UniformSpec(obj.name, type));
}

Uniform Loader::ExtractUniform_(const Parser::Object &obj,
                                ShaderInputRegistry &reg) {
    // There has to be a name for the uniform.
    if (obj.name.empty())
        ThrowMissingName_(obj);

    // Look up the uniform in the registry.
    const ShaderInputRegistry::UniformSpec *spec = reg.Find<Uniform>(obj.name);
    if (! spec)
        throw Exception(obj, "No Uniform named '" + obj.name +
                        " found in shader registry");

    Uniform u;

    for (const Parser::FieldPtr &field_ptr: obj.fields) {
        const Parser::Field &field = *field_ptr;
        if      (field.spec.name == "float_val")
            u = reg.Create<Uniform>(obj.name, field.GetValue<float>());
        else if (field.spec.name == "int_val")
            u = reg.Create<Uniform>(obj.name, field.GetValue<int>());
        else if (field.spec.name == "uint_val")
            u = reg.Create<Uniform>(
                obj.name, static_cast<uint32_t>(field.GetValue<uint>()));
        else if (field.spec.name == "vec2f_val")
            u = reg.Create<Uniform>(obj.name, ToVector2f_(field));
        else if (field.spec.name == "vec3f_val")
            u = reg.Create<Uniform>(obj.name, ToVector3f_(field));
        else if (field.spec.name == "vec4f_val")
            u = reg.Create<Uniform>(obj.name, ToVector4f_(field));
        else if (field.spec.name == "vec2i_val")
            u = reg.Create<Uniform>(obj.name, ToVector2i_(field));
        else if (field.spec.name == "vec3i_val")
            u = reg.Create<Uniform>(obj.name, ToVector3i_(field));
        else if (field.spec.name == "vec4i_val")
            u = reg.Create<Uniform>(obj.name, ToVector4i_(field));
        else if (field.spec.name == "vec2ui_val")
            u = reg.Create<Uniform>(obj.name, ToVector2ui_(field));
        else if (field.spec.name == "vec3ui_val")
            u = reg.Create<Uniform>(obj.name, ToVector3ui_(field));
        else if (field.spec.name == "vec4ui_val")
            u = reg.Create<Uniform>(obj.name, ToVector4ui_(field));
        else if (field.spec.name == "mat2_val")
            u = reg.Create<Uniform>(obj.name, ToMatrix2f_(field));
        else if (field.spec.name == "mat3_val")
            u = reg.Create<Uniform>(obj.name, ToMatrix3f_(field));
        else if (field.spec.name == "mat4_val")
            u = reg.Create<Uniform>(obj.name, ToMatrix4f_(field));
        else
            ThrowBadField_(obj, field);
    }

    if (! u.IsValid())
        throw Exception(obj, "Missing or wrong value type for uniform '" +
                        obj.name + "'");
    return u;
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
            spec.size = ToVector3f_(field);
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
            spec.longitude_start = ToAnglef_(field);
        else if (field.spec.name == "longitude_end")
            spec.longitude_end = ToAnglef_(field);
        else if (field.spec.name == "latitude_start")
            spec.latitude_start = ToAnglef_(field);
        else if (field.spec.name == "latitude_end")
            spec.latitude_end = ToAnglef_(field);
        else if (field.spec.name == "band_count")
            spec.band_count = field.GetValue<int>();
        else if (field.spec.name == "sector_count")
            spec.sector_count = field.GetValue<int>();
        else if (field.spec.name == "size")
            spec.size = ToVector3f_(field);
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
            if (! ToEnum_<PlanarShapeSpec::PlaneNormal>(field,
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
            if (! ToEnum_<PlanarShapeSpec::PlaneNormal>(field,
                                                        spec.plane_normal))
                ThrowEnumException_(obj, field, "PlanarShapeSpec::PlaneNormal");
        }
        else if (field.spec.name == "size")
            spec.size = ToVector2f_(field);
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
    throw Exception(obj, "Expected " + expected_type +
                    " object, got " + obj.spec.type_name);
}

void Loader::ThrowMissingName_(const Parser::Object &obj) {
    throw Exception(obj, "Object of type '" + obj.spec.type_name +
                    " must have a name");
}

void Loader::ThrowMissingField_(const Parser::Object &obj,
                                const std::string &field_name) {
    throw Exception(obj, "Missing required field '" + field_name +
                    "' in " + obj.spec.type_name + " object");
}

void Loader::ThrowBadField_(const Parser::Object &obj,
                            const Parser::Field &field) {
    throw Exception(obj, "Invalid field '" + field.spec.name +
                    "' found in " + obj.spec.type_name + " object");
}

void Loader::ThrowEnumException_(const Parser::Object &obj,
                                 const Parser::Field &field,
                                 const std::string &enum_type_name) {
    throw Exception(obj, "Invalid value string '" +
                    field.GetValue<std::string>() + "' for enum of type " +
                    enum_type_name);
}
