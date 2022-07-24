#include "Items/SessionState.h"

void SessionState::AddFields() {
    AddField(point_target_visible_.Init("point_target_visible", false));
    AddField(edge_target_visible_.Init("edge_target_visible",   false));
    AddField(edges_shown_.Init("edges_shown",                   false));
    AddField(build_volume_visible_.Init("build_volume_visible", false));
    AddField(axis_aligned_.Init("axis_aligned",                 false));

    Parser::Object::AddFields();
}

bool SessionState::IsSameAs(const SessionState &other) const {
    return
        point_target_visible_ == other.point_target_visible_ &&
        edge_target_visible_  == other.edge_target_visible_  &&
        edges_shown_          == other.edges_shown_          &&
        build_volume_visible_ == other.build_volume_visible_ &&
        axis_aligned_         == other.axis_aligned_;
}
