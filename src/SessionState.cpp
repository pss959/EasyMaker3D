#include "SessionState.h"

void SessionState::AddFields() {
    Parser::Object::AddFields();
    AddField(point_target_visible_);
    AddField(edge_target_visible_);
    AddField(edges_shown_);
    AddField(build_volume_visible_);
    AddField(axis_aligned_);
}
