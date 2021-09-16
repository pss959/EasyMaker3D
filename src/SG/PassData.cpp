#include "SG/PassData.h"

namespace SG {

void PassData::AddFields() {
    AddField(shader_name_);
    AddField(uniform_block_);
}

}  // namespace SG
