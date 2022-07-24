#include "Items/GripGuide.h"

void GripGuide::AddFields() {
    AddField(guide_type_.Init("guide_type"));
    AddField(hover_point_.Init("hover_point", Point3f::Zero()));

    SG::Node::AddFields();
}
