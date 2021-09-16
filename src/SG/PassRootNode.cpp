#include "SG/PassRootNode.h"

namespace SG {

void PassRootNode::AddFields() {
    Node::AddFields();
    AddField(default_shader_name_);
}

}  // namespace SG
