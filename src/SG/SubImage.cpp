#include "SG/SubImage.h"

namespace SG {

void SubImage::AddFields() {
    AddField(texture_scale_);
    AddField(texture_offset_);
    Object::AddFields();
}

}  // namespace SG
