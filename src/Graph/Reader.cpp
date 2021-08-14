#include "Graph/Reader.h"

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

    //! Reads an Ion node subgraph from a file specified by full path,
    //! returning an Ion NodePtr to the result.
    NodePtr ReadNode(const Util::FilePath &path);

    ImagePtr ReadImage(const Util::FilePath &path);

  private:
    IResourceManager &resource_manager_;

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

    // Determine the ShaderInputRegistry to use for uniforms. If the node has a
    // ShaderProgram, use its registry. Otherwise, use the one passed from
    // above, if it is not null. Otherwise, use the global registry.
    ShaderProgramPtr shader_program = node->GetIonNode()->GetShaderProgram();
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
