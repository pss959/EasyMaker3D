#pragma once

#include <memory>
#include <vector>

#include "Graph/Camera.h"
#include "Graph/Typedefs.h"
#include "Parser/Typedefs.h"

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
    //! find resources so they are not loaded unnecessarily.
    Extractor(Tracker &tracker_);
    ~Extractor();

    //! Extracts a Graph::Scene from a parsed graph.
    Graph::ScenePtr ExtractScene(const Parser::Object &obj);

  private:
    //! Tracker instance used to track resources to avoid extra loading.
    Tracker &tracker_;

    //! Manages a stack of currently-open Graph::Node instances so that they
    //! can be searched for shaders. It's a vector so all instances are
    //! accessible.
    std::vector<Graph::NodePtr> node_stack_;

    Graph::Camera ExtractCamera_(const Parser::Object &obj);
    Graph::NodePtr ExtractNode_(const Parser::Object &obj);
#if XXXX
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
#endif

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
