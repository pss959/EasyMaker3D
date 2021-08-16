#include "Input/Extractor.h"

#include <unordered_map>
#include <vector>

#include <ion/gfx/sampler.h>
#include <ion/gfx/shaderinputregistry.h>
#include <ion/gfx/shape.h>
#include <ion/gfxutils/shadermanager.h>
#include <ion/gfxutils/shadersourcecomposer.h>
#include <ion/gfxutils/shapeutils.h>

#include "ExceptionBase.h"
#include "Graph/Box.h"
#include "Graph/Cylinder.h"
#include "Graph/Ellipsoid.h"
#include "Graph/Image.h"
#include "Graph/Node.h"
#include "Graph/Polygon.h"
#include "Graph/Rectangle.h"
#include "Graph/Sampler.h"
#include "Graph/Scene.h"
#include "Graph/ShaderProgram.h"
#include "Graph/ShaderSource.h"
#include "Graph/Shape.h"
#include "Graph/Texture.h"
#include "Input/Conversion.h"
#include "Input/Exception.h"
#include "Input/Tracker.h"
#include "Parser/Parser.h"
#include "Parser/Typedefs.h"
#include "Util/Enum.h"
#include "Util/FilePath.h"
#include "Util/General.h"
#include "Util/Read.h"

using ion::gfx::ShaderInputRegistry;
using ion::gfx::ShaderInputRegistryPtr;
using ion::gfx::Uniform;
using ion::gfxutils::PlanarShapeSpec;
using ion::gfxutils::StringComposer;

namespace Input {

// ----------------------------------------------------------------------------
// Extractor::InstanceMap_ class.
// ----------------------------------------------------------------------------

//! This class is used to detect and track Parser::Object instances so that
//! they result in Graph::Object instances.
class Extractor::InstanceMap_ {
  public:
    //! Convenience typedef for error function.
    typedef std::function<void(const Parser::Object&,
                               const std::string &)> ErrorFunc;

    //! Sets the error function to invoke in FindOrCreate().
    void SetErrorFunc(const ErrorFunc &error_func) {
        error_func_ = error_func_;
    }

    //! If there is already a derived Graph::Object of the templated type
    //! associated with the given Parser::Object, this returns it. Otherwise,
    //! it returns a null pointer.
    template <typename T>
    std::shared_ptr<T> Find(const Parser::Object &parsed_obj) {
        return Util::CastToDerived<Graph::Object, T>(Find_(parsed_obj));
    }

    //! If there is already a derived Graph::Object of the templated type
    //! associated with the given Parser::Object, this sets graph_obj to point
    //! to it and returns true. Otherwise, it creates and adds a new
    //! Graph::Object of the correct type, storing the pointer in graph_obj. If
    //! the Parser::Object has the wrong type (as specified by type_name), the
    //! error function is invoked and nothing else is done.
    template <typename T>
    bool FindOrCreate(const Parser::Object &parsed_obj,
                      std::shared_ptr<T> &graph_obj,
                      const std::string &type_name) {
        // If an instance exists, return it.
        graph_obj = Find<T>(parsed_obj);
        if (graph_obj)
            return true;

        // Validate the type.
        if (parsed_obj.spec.type_name != type_name) {
            if (error_func_)
                error_func_(parsed_obj, type_name);
        }
        else {
            // Create the object and set its name.
            graph_obj.reset(new T);
            graph_obj->SetName_(parsed_obj.name);

            // Add to the map.
            Add_(parsed_obj, Util::CastToBase<Graph::Object, T>(graph_obj));
        }
        return false;
    }

  private:
    ErrorFunc error_func_;
    std::unordered_map<const Parser::Object *, Graph::ObjectPtr> map_;

    //! Finds and returns the associated Graph::Object for a Parser::Object, if
    //! there is one.
    Graph::ObjectPtr Find_(const Parser::Object &parsed_obj) {
        auto it = map_.find(&parsed_obj);
        if (it != map_.end())
            return it->second;
        return Graph::ObjectPtr();
    }

    //! Adds an association between a Graph::Object and a Parser::Object.
    void Add_(const Parser::Object &parsed_obj,
              const Graph::ObjectPtr &graph_obj) {
        assert(! Util::MapContains(map_, &parsed_obj));
        map_[&parsed_obj] = graph_obj;
    }
};

// ----------------------------------------------------------------------------
// Extractor functions
// ----------------------------------------------------------------------------

Extractor::Extractor(Tracker &tracker,
                     ion::gfxutils::ShaderManager &shader_manager) :
    tracker_(tracker),
    shader_manager_(shader_manager),
    instance_map_(new InstanceMap_) {

    // Set the InstanceMap_ error function to call ThrowTypeMismatch_().
    instance_map_->SetErrorFunc(
        [&](const Parser::Object &obj, const std::string &tn){
            ThrowTypeMismatch_(obj, tn);
        });
}

Extractor::~Extractor() {
}

Graph::ScenePtr Extractor::ExtractScene(const Parser::Object &obj) {
    CheckObjectType_(obj, "Scene");

    Graph::ScenePtr scene(new Graph::Scene);
    scene->SetName_(obj.name);

    for (const Parser::FieldPtr &field_ptr: obj.fields) {
        const Parser::Field &field = *field_ptr;
        if (field.spec.name == "camera")
            scene->SetCamera_(
                ExtractCamera_(*field.GetValue<Parser::ObjectPtr>()));
        else if (field.spec.name == "root")
            scene->SetRootNode_(
                ExtractNode_(*field.GetValue<Parser::ObjectPtr>()));
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
    Graph::NodePtr node;
    if (instance_map_->FindOrCreate<Graph::Node>(obj, node, "Node"))
        return node;

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
            node->SetEnabled(field.GetValue<bool>());
        else if (field.spec.name == "scale")
            node->SetScale(Conversion::ToVector3f(field));
        else if (field.spec.name == "rotation")
            node->SetRotation(Conversion::ToRotationf(field));
        else if (field.spec.name == "translation")
            node->SetTranslation(Conversion::ToVector3f(field));
        else if (field.spec.name == "state_table")
            node->SetStateTable_(
                ExtractStateTable_(*field.GetValue<Parser::ObjectPtr>()));
        else if (field.spec.name == "shader")
            node->SetShaderProgram_(
                ExtractShaderProgram_(*field.GetValue<Parser::ObjectPtr>()));
        else if (field.spec.name == "textures")
            for (const Parser::ObjectPtr &tex_obj:
                     field.GetValue<std::vector<Parser::ObjectPtr>>())
                node->AddTexture_(ExtractTexture_(*tex_obj));
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
    ion::gfx::ShaderProgramPtr program = FindShaderProgram_();
    ShaderInputRegistryPtr reg = program ? program->GetRegistry() :
        ShaderInputRegistry::GetGlobalRegistry();

    // Add texture uniforms.
    for (const auto &tex: node->GetTextures())
        node->AddUniform_(reg->Create<Uniform>(tex->GetUniformName(),
                                               tex->GetIonTexture()));

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
    // There has to be a name for the shader.
    if (obj.name.empty())
        ThrowMissingName_(obj);

    Graph::ShaderProgramPtr program;
    if (instance_map_->FindOrCreate<Graph::ShaderProgram>(obj, program,
                                                          "Shader"))
        return program;

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
    ion::gfxutils::ShaderManager::ShaderSourceComposerSet composer_set;
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

    ion::gfx::ShaderProgramPtr ion_program =
        shader_manager_.CreateShaderProgram(obj.name, reg, composer_set);
    if (! ion_program->GetInfoLog().empty())
        throw Exception(obj, "Unable to compile shader program: " +
                        ion_program->GetInfoLog());

    program->SetIonShaderProgram_(ion_program);

    return program;
}

Graph::TexturePtr Extractor::ExtractTexture_(const Parser::Object &obj) {
    Graph::TexturePtr texture;
    if (instance_map_->FindOrCreate<Graph::Texture>(obj, texture, "Texture"))
        return texture;

    for (const Parser::FieldPtr &field_ptr: obj.fields) {
        const Parser::Field &field = *field_ptr;
        if (field.spec.name == "uniform_name")
            texture->SetUniformName_(field.GetValue<std::string>());
        else if (field.spec.name == "image_file")
            texture->SetImage_(ExtractImage_(field));
        else if (field.spec.name == "sampler")
            texture->SetSampler_(
                ExtractSampler_(*field.GetValue<Parser::ObjectPtr>()));
        else
            ThrowBadField_(obj, field);
    }
    return texture;
}

Graph::SamplerPtr Extractor::ExtractSampler_(const Parser::Object &obj) {
    Graph::SamplerPtr sampler;
    if (instance_map_->FindOrCreate<Graph::Sampler>(obj, sampler, "Sampler"))
        return sampler;

    for (const Parser::FieldPtr &field_ptr: obj.fields) {
        const Parser::Field &field = *field_ptr;
        if (field.spec.name == "wrap_s_mode" ||
            field.spec.name == "wrap_t_mode") {
            ion::gfx::Sampler::WrapMode wrap_mode;
            if (! Conversion::ToEnum<ion::gfx::Sampler::WrapMode>(
                    field, wrap_mode))
                ThrowEnumException_(obj, field, "Sampler::WrapMode");
            if (field.spec.name == "wrap_s_mode")
                sampler->GetIonSampler()->SetWrapS(wrap_mode);
            else
                sampler->GetIonSampler()->SetWrapT(wrap_mode);
        }
        else
            ThrowBadField_(obj, field);
    }
    return sampler;
}

Graph::ShapePtr Extractor::ExtractShape_(const Parser::Object &obj) {
    Graph::ShapePtr shape = instance_map_->Find<Graph::Shape>(obj);

    if (! shape) {
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
        shape->SetName_(obj.name);
    }
    return shape;
}

Graph::ShapePtr Extractor::ExtractBox_(const Parser::Object &obj) {
    ion::gfxutils::BoxSpec spec;
    for (const Parser::FieldPtr &field_ptr: obj.fields) {
        const Parser::Field &field = *field_ptr;
        if (field.spec.name == "size")
            spec.size = Conversion::ToVector3f(field);
        else
            ThrowBadField_(obj, field);
    }
    ion::gfx::ShapePtr ion_shape = ion::gfxutils::BuildBoxShape(spec);
    return Graph::ShapePtr(new Graph::Box(ion_shape));
}

Graph::ShapePtr Extractor::ExtractCylinder_(const Parser::Object &obj) {
    ion::gfxutils::CylinderSpec spec;
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
    ion::gfx::ShapePtr ion_shape = ion::gfxutils::BuildCylinderShape(spec);
    return Graph::ShapePtr(new Graph::Cylinder(ion_shape));
}

Graph::ShapePtr Extractor::ExtractEllipsoid_(const Parser::Object &obj) {
    ion::gfxutils::EllipsoidSpec spec;
    for (const Parser::FieldPtr &field_ptr: obj.fields) {
        const Parser::Field &field = *field_ptr;
        if      (field.spec.name == "longitude_start")
            spec.longitude_start = Conversion::ToAnglef(field);
        else if (field.spec.name == "longitude_end")
            spec.longitude_end = Conversion::ToAnglef(field);
        else if (field.spec.name == "latitude_start")
            spec.latitude_start = Conversion::ToAnglef(field);
        else if (field.spec.name == "latitude_end")
            spec.latitude_end = Conversion::ToAnglef(field);
        else if (field.spec.name == "band_count")
            spec.band_count = field.GetValue<int>();
        else if (field.spec.name == "sector_count")
            spec.sector_count = field.GetValue<int>();
        else if (field.spec.name == "size")
            spec.size = Conversion::ToVector3f(field);
        else
            ThrowBadField_(obj, field);
    }
    ion::gfx::ShapePtr ion_shape = ion::gfxutils::BuildEllipsoidShape(spec);
    return Graph::ShapePtr(new Graph::Ellipsoid(ion_shape));
}

Graph::ShapePtr Extractor::ExtractPolygon_(const Parser::Object &obj) {
    ion::gfxutils::RegularPolygonSpec spec;
    for (const Parser::FieldPtr &field_ptr: obj.fields) {
        const Parser::Field &field = *field_ptr;
        if      (field.spec.name == "sides")
            spec.sides = field.GetValue<int>();
        else if (field.spec.name == "plane_normal") {
            if (! Conversion::ToEnum<PlanarShapeSpec::PlaneNormal>(
                    field, spec.plane_normal))
                ThrowEnumException_(obj, field, "PlanarShapeSpec::PlaneNormal");
        }
        else
            ThrowBadField_(obj, field);
    }
    ion::gfx::ShapePtr ion_shape = ion::gfxutils::BuildRegularPolygonShape(spec);
    return Graph::ShapePtr(new Graph::Polygon(ion_shape));
}

Graph::ShapePtr Extractor::ExtractRectangle_(const Parser::Object &obj) {
    ion::gfxutils::RectangleSpec spec;
    for (const Parser::FieldPtr &field_ptr: obj.fields) {
        const Parser::Field &field = *field_ptr;
        if      (field.spec.name == "plane_normal") {
            if (! Conversion::ToEnum<PlanarShapeSpec::PlaneNormal>(
                    field, spec.plane_normal))
                ThrowEnumException_(obj, field, "PlanarShapeSpec::PlaneNormal");
        }
        else if (field.spec.name == "size")
            spec.size = Conversion::ToVector2f(field);
        else
            ThrowBadField_(obj, field);
    }
    ion::gfx::ShapePtr ion_shape = ion::gfxutils::BuildRectangleShape(spec);
    return Graph::ShapePtr(new Graph::Rectangle(ion_shape));
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

Graph::ImagePtr Extractor::ExtractImage_(const Parser::Field &field) {
    // See if the image was already loaded.
    const std::string &path = Util::FilePath::GetResourcePath(
        "textures", field.GetValue<std::string>());
    Graph::ImagePtr image = tracker_.FindImage(path);
    if (! image) {
        ion::gfx::ImagePtr ion_image = Util::ReadImage(path);
        if (! ion_image)
            throw Exception(path, "Unable to open or read image file");
        image.reset(new Graph::Image(ion_image));
        image->SetFilePath_(path);
        tracker_.AddImage(image);
    }
    return image;
}

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
            ion::gfx::StateTable::Capability cap;
            if (! Conversion::ToEnum<ion::gfx::StateTable::Capability>(field,
                                                                       cap))
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
        else
            ThrowBadField_(obj, field);
    }

    if (! u.IsValid())
        throw Exception(obj, "Missing or wrong value type for uniform '" +
                        obj.name + "'");
    return u;
}

ion::gfx::ShaderProgramPtr Extractor::FindShaderProgram_() {
    // Look for a ShaderProgram in all current Nodes, starting at the top of
    // the stack (reverse iteration).
    for (auto it = std::rbegin(node_stack_);
         it != std::rend(node_stack_); ++it) {
        if ((*it)->GetShaderProgram())
            return (*it)->GetShaderProgram()->GetIonShaderProgram();
    }
    return ion::gfx::ShaderProgramPtr();  // Not found.
}

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
