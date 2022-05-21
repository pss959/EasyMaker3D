#include "Items/SessionState.h"

void SessionState::AddFields() {
    Parser::Object::AddFields();
    AddField(point_target_visible_);
    AddField(edge_target_visible_);
    AddField(edges_shown_);
    AddField(build_volume_visible_);
    AddField(axis_aligned_);
}

bool SessionState::IsSameAs(const SessionState &other) const {
    return
        point_target_visible_ == other.point_target_visible_ &&
        edge_target_visible_  == other.edge_target_visible_  &&
        edges_shown_          == other.edges_shown_          &&
        build_volume_visible_ == other.build_volume_visible_ &&
        axis_aligned_         == other.axis_aligned_;
}
