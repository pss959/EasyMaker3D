#include "Items/Shelf.h"

#include "Math/Linear.h"

void Shelf::Init(const SG::NodePtr &shelf_geometry,
                 const std::vector<WidgetPtr> &icons, float distance) {
    // A Shelf subgraph looks like this:
    //
    //   Shelf (named with the given name)
    //      "ShelfScaler" Node (scales width of shelf geometry)
    //         shelf_geometry
    //      "ShelfIcons" Node (parent for all icon Widgets)

    // Create, name, and add the geometry and icon parent nodes
    SG::NodePtr       scaler = SG::Node::Create("ShelfScaler");
    SG::NodePtr  icon_parent = SG::Node::Create("ShelfIcons");
    AddChild(scaler);
    AddChild(icon_parent);

    // Add the shelf geometry.
    scaler->AddChild(shelf_geometry);

    // Add the icons and get back the width to use for the shelf.
    const float new_shelf_width = AddIcons_(*icon_parent, icons, distance);

    // Scale the shelf to the correct size.
    const float cur_shelf_width = shelf_geometry->GetBounds().GetSize()[0];
    scaler->SetScale(Vector3f(new_shelf_width / cur_shelf_width, 1, 1));
}

float Shelf::AddIcons_(Node &parent, const std::vector<WidgetPtr> &icons,
                       float distance) {
    // Base size to use for all 3D icons that appear on shelves. All icons are
    // scaled to fit this size in all 3 dimensions
    const float kIconSize_ = 3.6f;

    // Multiplier for scaling icons proportional to distance from the camera to
    // keep the sizes relatively constant on the screen.
    const float kDistanceScale_ = .015f;

    // Margin around and between icons.
    const float kMargin_ = 1.f;

    // Compensate for the distance of the shelf from the camera.
    const float distance_scale = kDistanceScale_ * distance;

    // Compute the icon size and margin.
    const float icon_size = distance_scale * kIconSize_;
    const float margin    = distance_scale * kMargin_;

    // Scale each icon to the correct size, position it, and enable it.
    float x = margin + .5f * icon_size;
    for (auto &icon: icons) {
        const Bounds bounds = icon->GetBounds();
        const int min_index = GetMinElementIndex(bounds.GetSize());
        const float scale = icon_size / bounds.GetSize()[min_index];
        icon->SetScale(scale * icon->GetScale());
        icon->SetTranslation(Vector3f(x, .5f * icon_size, 0));
        x += icon_size + margin;

        // Enable the icon and add it as a child of the parent.
        icon->SetEnabled(SG::Node::Flag::kTraversal, true);
        parent.AddChild(icon);
    }

    // Return the shelf width based on the number of icons.
    const size_t icon_count = icons.size();
    return icon_count * icon_size + (icon_count + 1) * margin;
}
