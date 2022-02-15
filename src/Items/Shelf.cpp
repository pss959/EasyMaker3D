#include "Items/Shelf.h"

#include "Defaults.h"
#include "Math/Linear.h"
#include "SG/Search.h"
#include "Util/General.h"

void Shelf::AddFields() {
    AddField(depth_scale_);
    AddField(icons_);
    SG::Node::AddFields();
}

void Shelf::CreationDone() {
    SG::Node::CreationDone();

    if (! IsTemplate()) {
        // Add all icons as children. Note that this must be done before
        // LayOutIcons() is called or the icon bounds will not be correct.
        for (auto &icon: GetIcons())
            AddChild(icon);
    }
}

void Shelf::LayOutIcons(const Point3f &cam_pos) {
    ASSERT(IsCreationDone());

    // Access the geometry child.
    auto geom = SG::FindNodeUnderNode(*this, "ShelfGeom");

    // Add the icons and get back the width to use for the shelf.
    Vector3f geom_size = geom->GetScaledBounds().GetSize();
    ASSERT(geom_size[0] > 0);

    // Adjust the depth.
    geom_size[2] *= depth_scale_;

    const float distance =
        ion::math::Distance(cam_pos, Point3f(GetTranslation()));
    const float new_shelf_width = AddIcons_(distance, geom_size[2]);

    // Scale the shelf geometry to the correct size.
    Vector3f scale = geom->GetScale();
    scale[0] *= new_shelf_width / geom_size[0];
    scale[2] *= depth_scale_;
    geom->SetScale(scale);

    // Translate the entire shelf forward or back to match the depth scale.
    Vector3f trans = GetTranslation();
    trans[2] += GetScale()[2] * (depth_scale_ - 1) * geom_size[2];
    SetTranslation(trans);
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
        // Position in Z so that the front of the icon is flush with the front
        // of the shelf.
        const float z = .5f * (shelf_depth - icon_size);
        icon->FitIntoCube(icon_size, Point3f(x, y, z));

        x += icon_size + margin;

        // Enable the widget and add it as a child.
        icon->SetEnabled(true);
    }

    return shelf_width;
}
