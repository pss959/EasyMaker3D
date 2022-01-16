#include "Items/Shelf.h"

#include "Defaults.h"
#include "Managers/ActionManager.h"
#include "Math/Linear.h"
#include "SG/Search.h"
#include "Util/General.h"

void Shelf::AddFields() {
    AddField(icons_);
    SG::Node::AddFields();
}

void Shelf::CreationDone(bool is_template) {
    // Add all icons as children. Note that this must be done before
    // LayOutIcons() is called or the icon bounds will not be correct.
    for (auto &icon: GetIcons())
        AddChild(icon);
    SG::Node::CreationDone(is_template);
}

void Shelf::LayOutIcons(const Point3f &cam_pos, ActionManager &action_manager) {
    // Set up all the icon widgets.
    for (const auto &icon: GetIcons()) {
        const Action action = icon->GetAction();
        icon->SetEnableFunction(
            std::bind(&ActionManager::CanApplyAction, &action_manager, action));
        icon->GetClicked().AddObserver(
            this, [action, &action_manager](const ClickInfo &info){
                action_manager.ApplyAction(action);
            });
    }

    // Access the geometry child.
    auto geom = SG::FindNodeUnderNode(*this, "ShelfGeom");

    // Add the icons and get back the width to use for the shelf.
    const Vector3f geom_size = geom->GetScaledBounds().GetSize();
    ASSERT(geom_size[0] > 0);
    const float distance =
        ion::math::Distance(cam_pos, Point3f(GetTranslation()));
    const float new_shelf_width = AddIcons_(distance, geom_size[2]);

    // Scale the shelf to the correct size.
    Vector3f scale = geom->GetScale();
    scale[0] *= new_shelf_width / geom_size[0];
    geom->SetScale(scale);
}

float Shelf::AddIcons_(float distance, float shelf_depth) {
    // Multiplier for scaling icons proportional to distance from the camera to
    // keep the sizes relatively constant on the screen.
    const float kDistanceScale_ = .015f;

    // Margin around and between icons.
    const float kMargin_ = 1.f;

    // Extra height for placing icons.
    const float kExtraHeight_ = 1.2f;

    // Compensate for the distance of the shelf from the camera.
    const float distance_scale = kDistanceScale_ * distance;

    // Compute the icon size and margin.
    const float icon_size = distance_scale * Defaults::kIconSize;
    const float margin    = distance_scale * kMargin_;

    // Compute the shelf width based on the number of icons.
    auto &icons = GetIcons();
    const size_t icon_count = icons.size();
    const float shelf_width =
        icon_count * icon_size + (icon_count + 1) * margin;

    // Position in X based on width and in Y based on height (plus a little).
    float x = -.5f * shelf_width + margin + .5f * icon_size;
    const float y = .5f * icon_size + kExtraHeight_;

    // Scale each icon to the correct size, position it, and enable it.
    for (auto &icon: GetIcons()) {
        const Bounds bounds = icon->GetBounds();
        const int max_index = GetMaxElementIndex(bounds.GetSize());
        const float scale = icon_size / bounds.GetSize()[max_index];
        icon->SetScale(scale * icon->GetScale());

        // Position in Z so that the front of the icon is just past the front
        // of the shelf.
        const float z = 2.f + .5f * (shelf_depth - scale * bounds.GetSize()[2]);

        icon->SetTranslation(Vector3f(x, y, z));
        x += icon_size + margin;

        // Enable the widget and add it as a child.
        icon->SetEnabled(SG::Node::Flag::kTraversal, true);
    }

    return shelf_width;
}
