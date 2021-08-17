#include "SG/Node.h"

#include "SG/SpecBuilder.h"

namespace SG {

std::vector<NParser::FieldSpec> Node::GetFieldSpecs_() {
    SG::SpecBuilder<Node> builder;
    builder.AddVector3f("scale",       &Node::scale_);
    builder.AddRotationf("rotation",   &Node::rotation_);
    builder.AddVector3f("translation", &Node::translation_);
    // XXXX builder.AddObject<ShaderProgram>("shader", &Node::shader_program_);
    return builder.GetSpecs();
}

}  // namespace SG
