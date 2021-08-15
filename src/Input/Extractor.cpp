#include "Input/Extractor.h"

#include <vector>
#include <vector>

#include <ion/gfx/sampler.h>
#include <ion/gfx/shaderinputregistry.h>
#include <ion/gfx/shape.h>
#include <ion/gfx/texture.h>
#include <ion/gfxutils/shadermanager.h>
#include <ion/gfxutils/shadersourcecomposer.h>
#include <ion/gfxutils/shapeutils.h>
#include <ion/image/conversionutils.h>
#include <ion/math/angle.h>
#include <ion/math/matrix.h>
#include <ion/math/rotation.h>
#include <ion/math/vector.h>
#include <ion/port/fileutils.h>

#include "ExceptionBase.h"
#include "Graph/Node.h"
#include "Graph/Scene.h"
#include "Graph/ShaderProgram.h"
#include "Graph/ShaderSource.h"
#include "Input/Conversion.h"
#include "Input/Exception.h"
#include "Input/Tracker.h"
#include "Parser/Object.h"
#include "Parser/Parser.h"
#include "Parser/Typedefs.h"
#include "Parser/Visitor.h"
#include "Util/Enum.h"
#include "Util/FilePath.h"
#include "Util/Read.h"

using ion::gfx::ImagePtr;
using ion::gfx::NodePtr;
using ion::gfx::SamplerPtr;
using ion::gfx::ShaderInputRegistry;
using ion::gfx::ShaderInputRegistryPtr;
using ion::gfx::ShaderProgramPtr;
using ion::gfx::ShapePtr;
using ion::gfx::StateTable;
using ion::gfx::TexturePtr;
using ion::gfx::Uniform;
using ion::gfxutils::PlanarShapeSpec;
using ion::gfxutils::ShaderManager;
using ion::gfxutils::StringComposer;
using ion::gfxutils::StringComposerPtr;
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

namespace Input {

// ----------------------------------------------------------------------------
// Public Extractor functions.
// ----------------------------------------------------------------------------

Extractor::Extractor(Tracker &tracker, ShaderManager &shader_manager) :
    tracker_(tracker), shader_manager_(shader_manager) {
}

Extractor::~Extractor() {
}

Graph::ScenePtr Extractor::ExtractScene(const Parser::Object &obj) {
    CheckObjectType_(obj, "Scene");

    Graph::ScenePtr scene(new Graph::Scene);

    for (const Parser::FieldPtr &field_ptr: obj.fields) {
        const Parser::Field &field = *field_ptr;
        if (field.spec.name == "camera")
            scene->SetCamera_(
                ExtractCamera_(*field.GetValue<Parser::ObjectPtr>()));
        else if (field.spec.name == "nodes")
            for (const Parser::ObjectPtr &node_obj:
                     field.GetValue<std::vector<Parser::ObjectPtr>>())
                scene->GetRootNode()->AddChild_(ExtractNode_(*node_obj));
        else
            ThrowBadField_(obj, field);
    }
    return scene;
}

Graph::Camera Extractor::ExtractCamera_(const Parser::Object &obj) {
    Graph::Camera camera;
    for (const Parser::FieldPtr &field_ptr: obj.fields) {
        const Parser::Field &field = *field_ptr;
        if (field.spec.name == "position")
            camera.position = Conversion::ToVector3f(field);
        else if (field.spec.name == "orientation")
            camera.orientation = Conversion::ToRotationf(field);
        else if (field.spec.name == "fov")
            // XXXX Use real aspect ratio?
            camera.fov = Graph::Camera::FOV(Conversion::ToAnglef(field), 1.f);
        else if (field.spec.name == "near")
            camera.near = field.GetValue<float>();
        else if (field.spec.name == "far")
            camera.far = field.GetValue<float>();
        else
            ThrowBadField_(obj, field);
    }
    return camera;
}

Graph::NodePtr Extractor::ExtractNode_(const Parser::Object &obj) {
    CheckObjectType_(obj, "Node");

    Graph::NodePtr node(new Graph::Node);
    node->SetName_(obj.name);
    node_stack_.push_back(node);

    // Saves Uniform objects to add after all fields are parsed, since the
    // shader must be known.
    std::vector<Parser::ObjectPtr> uniform_objs;

    // Saves child Node objects to add after all fields are parsed, since the
    // shader must be known.
    std::vector<Parser::ObjectPtr> child_objs;

    for (const Parser::FieldPtr &field_ptr: obj.fields) {
        const Parser::Field &field = *field_ptr;
        if      (field.spec.name == "enabled")
            node->SetEnabled_(field.GetValue<bool>());
        else if (field.spec.name == "scale")
            node->SetScale_(Conversion::ToVector3f(field));
        else if (field.spec.name == "rotation")
            node->SetRotation_(Conversion::ToRotationf(field));
        else if (field.spec.name == "translation")
            node->SetTranslation_(Conversion::ToVector3f(field));
        else if (field.spec.name == "state_table")
            node->SetStateTable_(
                ExtractStateTable_(*field.GetValue<Parser::ObjectPtr>()));
        else if (field.spec.name == "shader")
            node->SetShaderProgram_(
                ExtractShaderProgram_(*field.GetValue<Parser::ObjectPtr>()));
        else if (field.spec.name == "uniforms")
            uniform_objs = field.GetValue<std::vector<Parser::ObjectPtr>>();
        else if (field.spec.name == "shapes")
            for (const Parser::ObjectPtr &shape_obj:
                     field.GetValue<std::vector<Parser::ObjectPtr>>())
                node->AddShape_(ExtractShape_(*shape_obj));
        else if (field.spec.name == "children")
            child_objs = field.GetValue<std::vector<Parser::ObjectPtr>>();
        else
            ThrowBadField_(obj, field);
    }

    // Determine the ShaderInputRegistry to use for uniforms. Search up through
    // the Node stack (including this Node) for one. If none is found, use the
    // global registry.
    ShaderProgramPtr program = FindShaderProgram_();
    ShaderInputRegistryPtr reg = program ? program->GetRegistry() :
        ShaderInputRegistry::GetGlobalRegistry();

    // Add a matrix uniform if there was any transform.
    /* XXXX Figure this out!!!
    if (transform.AnyComponentSet())
        node->AddUniform(reg->Create<Uniform>("uModelviewMatrix",
                                              transform.GetMatrix()));
    */

    // Add any other uniforms.
    for (const Parser::ObjectPtr &uniform_obj: uniform_objs)
        node->AddUniform_(ExtractUniform_(*uniform_obj, *reg));

    // Process children.
    for (const Parser::ObjectPtr &child_obj: child_objs)
        node->AddChild_(ExtractNode_(*child_obj));

    assert(node_stack_.back() == node);
    node_stack_.pop_back();

    return node;
}

Graph::ShaderProgramPtr Extractor::ExtractShaderProgram_(
    const Parser::Object &obj) {
    CheckObjectType_(obj, "Shader");

    // There has to be a name for the shader.
    if (obj.name.empty())
        ThrowMissingName_(obj);

    Graph::ShaderProgramPtr program(new Graph::ShaderProgram);

    // Create a registry in case there are uniforms to be registered.
    ShaderInputRegistryPtr reg(new ShaderInputRegistry);
    reg->IncludeGlobalRegistry();

    for (const Parser::FieldPtr &field_ptr: obj.fields) {
        const Parser::Field &field = *field_ptr;
        if (field.spec.name == "uniform_defs") {
            for (const Parser::ObjectPtr &uniform_obj:
                     field.GetValue<std::vector<Parser::ObjectPtr>>())
                reg->Add<Uniform>(ExtractUniformSpec_(*uniform_obj));
        }
        else if (field.spec.name == "vertex_program")
            program->SetVertexSource_(ExtractShaderSource_(field));
        else if (field.spec.name == "geometry_program")
            program->SetGeometrySource_(ExtractShaderSource_(field));
        else if (field.spec.name == "fragment_program")
            program->SetFragmentSource_(ExtractShaderSource_(field));
        else
            ThrowBadField_(obj, field);
    }

    // Create a StringComposer for each supplied source and use them to create
    // the program.
    ShaderManager::ShaderSourceComposerSet composer_set;
    if (program->GetVertexSource())
        composer_set.vertex_source_composer =
            new StringComposer(obj.name + "_fp",
                               program->GetVertexSource()->GetSourceString());
    if (program->GetGeometrySource())
        composer_set.geometry_source_composer =
            new StringComposer(obj.name + "_gp",
                               program->GetGeometrySource()->GetSourceString());
    if (program->GetFragmentSource())
        composer_set.fragment_source_composer =
            new StringComposer(obj.name + "_gp",
                               program->GetFragmentSource()->GetSourceString());

    ShaderProgramPtr ion_program =
        shader_manager_.CreateShaderProgram(obj.name, reg, composer_set);
    if (! ion_program->GetInfoLog().empty())
        throw Exception(obj, "Unable to compile shader program: " +
                        ion_program->GetInfoLog());

    program->SetIonShaderProgram_(ion_program);

    return program;
}

Graph::ShaderSourcePtr Extractor::ExtractShaderSource_(
    const Parser::Field &field) {
    // See if the source was already loaded.
    const std::string &path = Util::FilePath::GetResourcePath(
        "shaders", field.GetValue<std::string>());
    Graph::ShaderSourcePtr source = tracker_.FindShaderSource(path);
    if (! source) {
        std::string source_str;
        if (! Util::ReadFile(path, source_str))
            throw Exception(path, "Unable to open or read contents of file");
        source.reset(new Graph::ShaderSource);
        source->SetFilePath_(path);
        source->SetSourceString_(source_str);
        tracker_.AddShaderSource(source);
    }
    return source;
}

// XXXX More Graph objects here...

ion::gfx::StateTablePtr Extractor::ExtractStateTable_(
    const Parser::Object &obj) {
    CheckObjectType_(obj, "StateTable");

    ion::gfx::StateTablePtr table(new ion::gfx::StateTable);

    for (const Parser::FieldPtr &field_ptr: obj.fields) {
        const Parser::Field &field = *field_ptr;

        if (field.spec.name == "clear_color")
            table->SetClearColor(Conversion::ToVector4f(field));
        else if (field.spec.name == "enable_cap" ||
                 field.spec.name == "disable_cap") {
            StateTable::Capability cap;
            if (! Conversion::ToEnum<StateTable::Capability>(field, cap))
                ThrowEnumException_(obj, field, "StateTable::Capability");
            table->Enable(cap, field.spec.name == "enable_cap");
        }
        else
            ThrowBadField_(obj, field);
    }
    return table;
}

ShaderInputRegistry::UniformSpec Extractor::ExtractUniformSpec_(
    const Parser::Object &obj) {
    // There has to be a name for the uniform.
    if (obj.name.empty())
        ThrowMissingName_(obj);

    Uniform::ValueType type;
    for (const Parser::FieldPtr &field_ptr: obj.fields) {
        const Parser::Field &field = *field_ptr;
        if (field.spec.name == "type") {
            if (! Conversion::ToEnum<Uniform::ValueType>(field, type))
                ThrowEnumException_(obj, field, "Uniform::ValueType");
        }
        else
            ThrowBadField_(obj, field);
    }

    ShaderInputRegistry::UniformSpec spec(obj.name, type);
    return ShaderInputRegistry::UniformSpec(obj.name, type);
}

Uniform Extractor::ExtractUniform_(const Parser::Object &obj,
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
            u = reg.Create<Uniform>(obj.name, Conversion::ToVector2f(field));
        else if (field.spec.name == "vec3f_val")
            u = reg.Create<Uniform>(obj.name, Conversion::ToVector3f(field));
        else if (field.spec.name == "vec4f_val")
            u = reg.Create<Uniform>(obj.name, Conversion::ToVector4f(field));
        else if (field.spec.name == "vec2i_val")
            u = reg.Create<Uniform>(obj.name, Conversion::ToVector2i(field));
        else if (field.spec.name == "vec3i_val")
            u = reg.Create<Uniform>(obj.name, Conversion::ToVector3i(field));
        else if (field.spec.name == "vec4i_val")
            u = reg.Create<Uniform>(obj.name, Conversion::ToVector4i(field));
        else if (field.spec.name == "vec2ui_val")
            u = reg.Create<Uniform>(obj.name, Conversion::ToVector2ui(field));
        else if (field.spec.name == "vec3ui_val")
            u = reg.Create<Uniform>(obj.name, Conversion::ToVector3ui(field));
        else if (field.spec.name == "vec4ui_val")
            u = reg.Create<Uniform>(obj.name, Conversion::ToVector4ui(field));
        else if (field.spec.name == "mat2_val")
            u = reg.Create<Uniform>(obj.name, Conversion::ToMatrix2f(field));
        else if (field.spec.name == "mat3_val")
            u = reg.Create<Uniform>(obj.name, Conversion::ToMatrix3f(field));
        else if (field.spec.name == "mat4_val")
            u = reg.Create<Uniform>(obj.name, Conversion::ToMatrix4f(field));
        else if (field.spec.name == "texture_val")
            /* XXXX
            u = reg.Create<Uniform>(
                obj.name,
                ExtractTexture_(*field.GetValue<Parser::ObjectPtr>()));
            */
            ;  // XXXX
        else
            ThrowBadField_(obj, field);
    }

    if (! u.IsValid())
        throw Exception(obj, "Missing or wrong value type for uniform '" +
                        obj.name + "'");
    return u;
}

#if XXXX

TexturePtr Extractor::ExtractTexture_(const Parser::Object &obj) {
    CheckObjectType_(obj, "Texture");
    TexturePtr texture(new ion::gfx::Texture);

    for (const Parser::FieldPtr &field_ptr: obj.fields) {
        const Parser::Field &field = *field_ptr;
        if (field.spec.name == "image_file")
            texture->SetImage(0U, resource_manager_.ReadTextureImage(
                                  field.GetValue<std::string>()));
        else if (field.spec.name == "sampler")
            texture->SetSampler(
                ExtractSampler_(*field.GetValue<Parser::ObjectPtr>()));
        else
            ThrowBadField_(obj, field);
    }
    return texture;
}

SamplerPtr Extractor::ExtractSampler_(const Parser::Object &obj) {
    CheckObjectType_(obj, "Sampler");
    SamplerPtr sampler(new ion::gfx::Sampler);

    for (const Parser::FieldPtr &field_ptr: obj.fields) {
        const Parser::Field &field = *field_ptr;
        if (field.spec.name == "wrap_s_mode" ||
            field.spec.name == "wrap_t_mode") {
            ion::gfx::Sampler::WrapMode wrap_mode;
            if (! Conversion::ToEnum<ion::gfx::Sampler::WrapMode>(
                    field, wrap_mode))
                ThrowEnumException_(obj, field, "Sampler::WrapMode");
            if (field.spec.name == "wrap_s_mode")
                sampler->SetWrapS(wrap_mode);
            else
                sampler->SetWrapT(wrap_mode);
        }
        else
            ThrowBadField_(obj, field);
    }
    return sampler;
}

ShaderProgramPtr Extractor::FindShaderProgram_() {
    // Look for a ShaderProgram in all current Nodes, starting at the top of
    // the stack (reverse iteration).
    for (auto it = std::rbegin(node_stack_);
         it != std::rend(node_stack_); ++it) {
        ShaderProgramPtr program = (*it)->GetIonNode()->GetShaderProgram();
        if (program)
            return program;
    }
    return ShaderProgramPtr();  // Not found.
}

#endif

void Extractor::CheckObjectType_(const Parser::Object &obj,
                                 const std::string &expected_type) {
    if (obj.spec.type_name != expected_type)
        ThrowTypeMismatch_(obj, expected_type);
}

void Extractor::ThrowTypeMismatch_(const Parser::Object &obj,
                                   const std::string &expected_type) {
    throw Exception(obj, "Expected " + expected_type +
                    " object, got " + obj.spec.type_name);
}

void Extractor::ThrowMissingName_(const Parser::Object &obj) {
    throw Exception(obj, "Object of type '" + obj.spec.type_name +
                    " must have a name");
}

void Extractor::ThrowMissingField_(const Parser::Object &obj,
                                   const std::string &field_name) {
    throw Exception(obj, "Missing required field '" + field_name +
                    "' in " + obj.spec.type_name + " object");
}

void Extractor::ThrowBadField_(const Parser::Object &obj,
                               const Parser::Field &field) {
    throw Exception(obj, "Invalid field '" + field.spec.name +
                    "' found in " + obj.spec.type_name + " object");
}

void Extractor::ThrowEnumException_(const Parser::Object &obj,
                                    const Parser::Field &field,
                                    const std::string &enum_type_name) {
    throw Exception(obj, "Invalid value string '" +
                    field.GetValue<std::string>() + "' for enum of type " +
                    enum_type_name);
}

}  // namespace Input
