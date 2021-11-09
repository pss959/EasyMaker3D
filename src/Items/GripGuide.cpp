#include "Items/GripGuide.h"

void GripGuide::AddFields() {
    AddField(guide_type_);
    AddField(hover_point_);
    SG::Node::AddFields();
}
