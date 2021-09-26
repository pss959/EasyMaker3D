#include "Managers/IconManager.h"

#include "Math/Linear.h"

/// Base size to use for all 3D icons that appear on shelves. All icons are
/// scaled to fit this size in all 3 dimensions
static const float kIconSize_ = 3.6f;

/// Multiplier for scaling icons proportional to distance from the camera to
/// keep the sizes relatively constant on the screen.
static const float kDistanceScale_ = .015f;

/// Margin around and between icons.
static const float kMargin_ = 1.f;

void IconManager::AddShelf(SG::Node &shelf,
                           const std::vector<WidgetPtr> &widgets) {
    // Compensate for the distance of the shelf from the camera.
    const float distance_scale = 1; // XXXX GetShelfDistanceScale(shelf);

    // Compute the icon size and margin.
    const float icon_size = distance_scale * kIconSize_;
    const float margin    = distance_scale * kMargin_;

    // XXXX Need to figure out how to scale the shelf without scaling the icons
    // on it!!

    // Resize the shelf width based on the number of icons.
    const size_t icon_count = widgets.size();
    const float cur_shelf_width = shelf.GetBounds().GetSize()[0];
    const float new_shelf_width =
        icon_count * icon_size + (icon_count + 1) * margin;
    Vector3f shelf_scale = shelf.GetScale();
    shelf_scale[0] = new_shelf_width / cur_shelf_width;
    shelf.SetScale(shelf_scale);

    // Scale each icon to the correct size, position it, and enable it.
    float x = margin + .5f * icon_size;
    for (auto &widget: widgets) {
        const Bounds bounds = widget->GetBounds();
        const int min_index = GetMinElementIndex(bounds.GetSize());
        const float scale = icon_size / bounds.GetSize()[min_index];
        widget->SetScale(scale * widget->GetScale());
        widget->SetTranslation(Vector3f(x, .5f * icon_size, 0));
        x += icon_size + margin;

        // Enable the icon and add it as a child of the shelf.
        widget->SetEnabled(SG::Node::Flag::kTraversal, true);
        shelf.AddChild(widget);
    }
}
