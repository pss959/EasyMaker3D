#include "SG/ShaderNode.h"

namespace SG {

void ShaderNode::AddFields() {
    AddField(pass_name_);
    AddField(shader_name_);
    Node::AddFields();
}

}  // namespace SG
