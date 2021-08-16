#pragma once

#include <memory>
#include <vector>

#include <ion/gfx/shaderinputregistry.h>
#include <ion/gfx/shaderprogram.h>
#include <ion/gfx/statetable.h>
#include <ion/gfx/uniform.h>
#include <ion/gfxutils/shadermanager.h>
#include <ion/gfxutils/shadersourcecomposer.h>

#include "Graph/Camera.h"
#include "Graph/Typedefs.h"
#include "Parser/Object.h"
#include "Parser/Typedefs.h"
#include "Util/General.h"

namespace Input {

class Tracker;

//! The Extractor class is used to extract Graph objects from a parsed graph.
//! Any failure results in an Input::Exception being thrown.
//
//! Note that this class is a friend of many of the Graph::Object types so that
//! it can modify instances of them.
//!
//! \ingroup Input
class Extractor {
  public:
    //! The constructor is passed a Tracker instance that is used to add and
    //! find resources so they are not loaded unnecessarily and an Ion
    //! ShaderManager used to create shaders.
    Extractor(Tracker &tracker_, ion::gfxutils::ShaderManager &shader_manager);
    ~Extractor();

    //! Extracts a Graph::Scene from a parsed graph.
    Graph::ScenePtr ExtractScene(const Parser::Object &obj);

  private:
    class InstanceMap_;

    //! Tracker instance used to track resources to avoid extra loading.
    Tracker &tracker_;

    //! ShaderManager used to create shaders.
    ion::gfxutils::ShaderManager &shader_manager_;

    //! This is used to track instances in the parsed data so they will result
    //! in instances in the graph.
    std::unique_ptr<InstanceMap_> instance_map_;

    //! Manages a stack of currently-open Graph::Node instances so that they
    //! can be searched for shaders. It's a vector so all instances are
    //! accessible.
    std::vector<Graph::NodePtr> node_stack_;

    // Extracting Graph objects from Parser Objects.
    Graph::Camera ExtractCamera_(const Parser::Object &obj);
    Graph::NodePtr ExtractNode_(const Parser::Object &obj);
    Graph::ShaderProgramPtr ExtractShaderProgram_(const Parser::Object &obj);
    Graph::TexturePtr ExtractTexture_(const Parser::Object &obj);
    Graph::SamplerPtr ExtractSampler_(const Parser::Object &obj);
    Graph::ShapePtr ExtractShape_(const Parser::Object &obj);
    Graph::ShapePtr ExtractBox_(const Parser::Object &obj);
    Graph::ShapePtr ExtractCylinder_(const Parser::Object &obj);
    Graph::ShapePtr ExtractEllipsoid_(const Parser::Object &obj);
    Graph::ShapePtr ExtractPolygon_(const Parser::Object &obj);
    Graph::ShapePtr ExtractRectangle_(const Parser::Object &obj);

    // Extracting Graph objects from Parser Fields.
    Graph::ShaderSourcePtr ExtractShaderSource_(const Parser::Field &field);
    Graph::ImagePtr ExtractImage_(const Parser::Field &field);

    // Extracting Ion objects.
    ion::gfx::StateTablePtr ExtractStateTable_(const Parser::Object &obj);
    ion::gfx::ShaderInputRegistry::UniformSpec ExtractUniformSpec_(
        const Parser::Object &obj);
    ion::gfx::Uniform ExtractUniform_(const Parser::Object &obj,
                                      ion::gfx::ShaderInputRegistry &reg);

    //! Searches upward in the Node stack for one with a ShaderProgram,
    //! returning the program.
    ion::gfx::ShaderProgramPtr FindShaderProgram_();

    //! Creates an Ion StringComposer to build a shader using the source from
    //! the path in the given field. The given name is used for the shader.
    ion::gfxutils::StringComposerPtr CreateStringComposer_(
        const std::string &name, const Parser::Field &field);

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

}  // namespace Input
