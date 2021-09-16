#include "SG/ShaderNode.h"

namespace SG {

void ShaderNode::AddFields() {
    Node::AddFields();
    AddField(shader_name_);
}

}  // namespace SG
