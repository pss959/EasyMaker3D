#include "SG/ShaderNode.h"

namespace SG {

void ShaderNode::AddFields() {
    Node::AddFields();
    AddField(pass_name_);
    AddField(shader_name_);
}

}  // namespace SG
