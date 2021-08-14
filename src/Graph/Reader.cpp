#include "Graph/Reader.h"

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
#include "Graph/Conversion.h"
#include "Graph/Node.h"
#include "Graph/Scene.h"
#include "Interfaces/IResourceManager.h"
#include "Parser/Object.h"
#include "Parser/Parser.h"
#include "Parser/Typedefs.h"
#include "Parser/Visitor.h"
#include "Util/Enum.h"
#include "Util/FilePath.h"

using ion::gfx::ImagePtr;
using ion::gfx::NodePtr;
using ion::gfx::SamplerPtr;
using ion::gfx::ShaderInputRegistry;
using ion::gfx::ShaderInputRegistryPtr;
using ion::gfx::ShaderProgramPtr;
using ion::gfx::ShapePtr;
using ion::gfx::StateTable;
using ion::gfx::StateTablePtr;
using ion::gfx::TexturePtr;
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

namespace Graph {

// ----------------------------------------------------------------------------
// Internal Reader_ class definition.
// ----------------------------------------------------------------------------

//! The Reader_ class implements the Reader functions. Note that this class is
//! a friend of many of the Graph::Object types so that it can modify instances
//! of them.
class Reader_ {
  public:
    //! Exception thrown when any reading function fails.
    class Exception : public ExceptionBase {
      public:
        Exception(const Util::FilePath &path, const std::string &msg) :
            ExceptionBase(path, "Error reading: " + msg) {}
        Exception(const Parser::Object &obj, const std::string &msg) :
            ExceptionBase(obj.path, obj.line_number, "Error reading: " + msg) {}
    };

    Reader_(IResourceManager &resource_manager);
    ~Reader_();

    //! Reads the contents of the file with the given path into a string and
    //! returns it.
    std::string ReadFile(const Util::FilePath &path);

    //! Reads a scene into the given Scene instance.
    void ReadScene(Scene &scene);

  private:
    IResourceManager &resource_manager_;

    //! Manages a stack of currently-open Node instances so that they can be
    //! searched for shaders. Use a vector so all instances are accessible.
    std::vector<NodePtr> node_stack_;

    //! Uses a Parser to parse the given file.
    Parser::ObjectPtr ParseFile_(const Util::FilePath &path);

    //! Adds file dependencies based on included paths in the parser graph
    //! rooted by the given object.
    void AddFileDependencies_(const Parser::Object &obj) const;

    // XXXX Parser object extraction functions.

    //! The scene extraction function is passed the Scene that is to be filled
    //! with the new contents. This allows the Scene to maintain the same root
    //! node across rereads.
    void ExtractScene_(const Parser::Object &obj, Scene &scene);
    NodePtr ExtractNode_(const Parser::Object &obj);
    Camera ExtractCamera_(const Parser::Object &obj);
    ion::gfx::StateTablePtr ExtractStateTable_(const Parser::Object &obj);
    ion::gfx::ShaderProgramPtr ExtractShaderProgram_(const Parser::Object &obj);
    //! Special case for UniformDef, which is passed the ShaderInputRegistry to
    //! add it to.
    void ExtractAndAddUniformDef_(const Parser::Object &obj,
                                  ion::gfx::ShaderInputRegistry &reg);
    ion::gfx::Uniform       ExtractUniform_(const Parser::Object &obj,
                                            ion::gfx::ShaderInputRegistry &reg);
    ion::gfx::TexturePtr    ExtractTexture_(const Parser::Object &obj);
    ion::gfx::SamplerPtr    ExtractSampler_(const Parser::Object &obj);
    ShapePtr ExtractShape_(const Parser::Object &obj);

    ion::gfx::ShapePtr      ExtractBox_(const Parser::Object &obj);
    ion::gfx::ShapePtr      ExtractCylinder_(const Parser::Object &obj);
    ion::gfx::ShapePtr      ExtractEllipsoid_(const Parser::Object &obj);
    ion::gfx::ShapePtr      ExtractPolygon_(const Parser::Object &obj);
    ion::gfx::ShapePtr      ExtractRectangle_(const Parser::Object &obj);

    //! Searches upward in the node_stack_ for one with a ShaderProgram.
    ShaderProgramPtr FindShaderProgram_();

    void CheckObjectType_(const Parser::Object &obj,
                          const std::string &expected_type);

    void ThrowTypeMismatch_(const Parser::Object &obj,
                            const std::string &expected_type);
    void ThrowMissingName_(const Parser::Object &obj);
    void ThrowMissingField_(const Parser::Object &obj,
                            const std::string &field_name);
    void ThrowBadField_(const Parser::Object &obj, const Parser::Field &field);
    void ThrowEnumException_(const Parser::Object &obj,
                             const Parser::Field &field,
                             const std::string &enum_type_name);
};

// ----------------------------------------------------------------------------
// Reader_ implementation.
// ----------------------------------------------------------------------------

// This file contains Parser specifications.
#include "Graph/ReaderSpecs.h"

Reader_::Reader_(IResourceManager &resource_manager) :
    resource_manager_(resource_manager) {
}

Reader_::~Reader_() {
    // Just in case.
    node_stack_.clear();
}

void Reader_::ReadScene(Scene &scene) {
    // Clear out old stuff.
    scene.ResetCamera();
    scene.GetRootNode()->ClearChildren_();
    ExtractScene_(*ParseFile_(scene.GetPath()), scene);
}

Parser::ObjectPtr Reader_::ParseFile_(const Util::FilePath &path) {
    Parser::ObjectPtr root;
    try {
        Parser::Parser parser(node_specs_);
        parser.SetBasePath(resource_manager_.GetBasePath());
        root = parser.ParseFile(path);
        // Tell the IResourceManager about included file dependencies.
        if (root.get())
            AddFileDependencies_(*root);
    }
    catch (Parser::Exception &ex) {
        throw Exception(path, std::string("Parsing failed:\n") + ex.what());
    }
    return root;
}

void Reader_::AddFileDependencies_(const Parser::Object &obj) const {
    Parser::Visitor::ObjectFunc func = [this](const Parser::Object &obj){
        for (const std::string &p: obj.included_paths)
            this->resource_manager_.AddDependency(obj.path, p);
    };
    Parser::Visitor::VisitObjects(obj, func);
}

void Reader_::ExtractScene_(const Parser::Object &obj, Scene &scene) {
    CheckObjectType_(obj, "Scene");

    for (const Parser::FieldPtr &field_ptr: obj.fields) {
        const Parser::Field &field = *field_ptr;
        if (field.spec.name == "camera")
            scene.SetCamera_(
                ExtractCamera_(*field.GetValue<Parser::ObjectPtr>()));
        else if (field.spec.name == "nodes")
            for (const Parser::ObjectPtr &node_obj:
                     field.GetValue<std::vector<Parser::ObjectPtr>>())
                scene.GetRootNode()->AddChild_(ExtractNode_(*node_obj));
        else
            ThrowBadField_(obj, field);
    }
}

NodePtr Reader_::ExtractNode_(const Parser::Object &obj) {
    CheckObjectType_(obj, "Node");

    NodePtr node(new Node);
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

StateTablePtr Reader_::ExtractStateTable_(const Parser::Object &obj) {
    CheckObjectType_(obj, "StateTable");

    StateTablePtr table(new StateTable);

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

ShaderProgramPtr Reader_::ExtractShaderProgram_(const Parser::Object &obj) {
    CheckObjectType_(obj, "Shader");

    ShaderManager::ShaderSourceComposerSet composer_set;

    // There has to be a name for the shader.
    if (obj.name.empty())
        ThrowMissingName_(obj);

    // Lambda for StringComposer builder to make things simpler.
    auto scb = [this, obj](const char *suffix, const Parser::Field &field){
        std::string source =
            resource_manager_.LoadShaderSource(field.GetValue<std::string>());
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
        resource_manager_.GetShaderManager().CreateShaderProgram(
            obj.name, reg, composer_set);
    if (! program->GetInfoLog().empty())
        throw Exception(obj, "Unable to compile shader program: " +
                        program->GetInfoLog());
    return program;
}

void Reader_::ExtractAndAddUniformDef_(const Parser::Object &obj,
                                       ShaderInputRegistry &reg) {
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
    reg.Add<Uniform>(ShaderInputRegistry::UniformSpec(obj.name, type));
}

Uniform Reader_::ExtractUniform_(const Parser::Object &obj,
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
            u = reg.Create<Uniform>(
                obj.name,
                ExtractTexture_(*field.GetValue<Parser::ObjectPtr>()));
        else
            ThrowBadField_(obj, field);
    }

    if (! u.IsValid())
        throw Exception(obj, "Missing or wrong value type for uniform '" +
                        obj.name + "'");
    return u;
}

TexturePtr Reader_::ExtractTexture_(const Parser::Object &obj) {
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

SamplerPtr Reader_::ExtractSampler_(const Parser::Object &obj) {
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

ShaderProgramPtr Reader_::FindShaderProgram_() {
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

void Reader_::CheckObjectType_(const Parser::Object &obj,
                               const std::string &expected_type) {
    if (obj.spec.type_name != expected_type)
        ThrowTypeMismatch_(obj, expected_type);
}

void Reader_::ThrowTypeMismatch_(const Parser::Object &obj,
                                 const std::string &expected_type) {
    throw Exception(obj, "Expected " + expected_type +
                    " object, got " + obj.spec.type_name);
}

void Reader_::ThrowMissingName_(const Parser::Object &obj) {
    throw Exception(obj, "Object of type '" + obj.spec.type_name +
                    " must have a name");
}

void Reader_::ThrowMissingField_(const Parser::Object &obj,
                                 const std::string &field_name) {
    throw Exception(obj, "Missing required field '" + field_name +
                    "' in " + obj.spec.type_name + " object");
}

void Reader_::ThrowBadField_(const Parser::Object &obj,
                             const Parser::Field &field) {
    throw Exception(obj, "Invalid field '" + field.spec.name +
                    "' found in " + obj.spec.type_name + " object");
}

void Reader_::ThrowEnumException_(const Parser::Object &obj,
                                  const Parser::Field &field,
                                  const std::string &enum_type_name) {
    throw Exception(obj, "Invalid value string '" +
                    field.GetValue<std::string>() + "' for enum of type " +
                    enum_type_name);
}

// ----------------------------------------------------------------------------
// Public Reader functions.
// ----------------------------------------------------------------------------

Reader::Reader(IResourceManager &resource_manager) :
    resource_manager_(resource_manager) {
}

Reader::~Reader() {
}

void Reader::ReadScene(Scene &scene) {
    Reader_(resource_manager_).ReadScene(scene);
}

}  // namespace Graph
