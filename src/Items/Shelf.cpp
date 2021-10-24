#include "Items/Shelf.h"

#include "Defaults.h"
#include "Managers/ActionManager.h"
#include "Math/Linear.h"
#include "SG/Search.h"
#include "Util/General.h"
#include "Widgets/IconWidget.h"

void Shelf::AddFields() {
    AddField(icons_);
    SG::Node::AddFields();
}

std::vector<IconPtr> Shelf::Init(const SG::NodePtr &shelf_geometry,
                                 const SG::NodePtr &icon_root,
                                 const Point3f &cam_pos,
                                 ActionManager &action_manager) {
    // A Shelf subgraph looks like this:
    //
    //   Shelf (named with the given name)
    //      "ShelfScaler" Node (scales width of shelf_geometry)
    //         "ShelfGeometry" (shelf_geometry)
    //      "ShelfIcons" Node (parent for all icon Widgets)
    //         ... Icons

    // Set up all the icon widgets.
    auto &icons = GetIcons();
    for (const auto &icon: icons) {
        IconWidgetPtr widget =
            SG::FindTypedNodeUnderNode<IconWidget>(*icon_root, icon->GetName());
        const Action action = icon->GetAction();
        widget->SetEnableFunction(
            std::bind(&ActionManager::CanApplyAction, &action_manager, action));
        widget->GetClicked().AddObserver(
            this, [action, &action_manager](const ClickInfo &info){
                action_manager.ApplyAction(action);
            });
        icon->SetWidget(widget);
    }

    // Create, name, and add the geometry and icon parent nodes
    SG::NodePtr       scaler = SG::Node::Create("ShelfScaler");
    SG::NodePtr  icon_parent = SG::Node::Create("ShelfIcons");
    AddChild(scaler);
    AddChild(icon_parent);

    // Add the shelf geometry.
    scaler->AddChild(shelf_geometry);

    // Add the icons and get back the width to use for the shelf.
    const float distance =
        ion::math::Distance(cam_pos, Point3f(GetTranslation()));
    const float shelf_depth = shelf_geometry->GetBounds().GetSize()[2];
    const float new_shelf_width =
        AddIcons_(*icon_parent, icons, distance, shelf_depth);

    // Scale the shelf to the correct size.
    const float cur_shelf_width =
        shelf_geometry->GetScaledBounds().GetSize()[0];
    scaler->SetScale(Vector3f(new_shelf_width / cur_shelf_width, 1, 1));

    return icons;
}

float Shelf::AddIcons_(Node &parent, const std::vector<IconPtr> &icons,
                       float distance, float shelf_depth) {
    // Multiplier for scaling icons proportional to distance from the camera to
    // keep the sizes relatively constant on the screen.
    const float kDistanceScale_ = .015f;

    // Margin around and between icons.
    const float kMargin_ = 2.f;

    // Extra height for placing icons.
    const float kExtraHeight_ = 1.2f;

    // Compensate for the distance of the shelf from the camera.
    const float distance_scale = kDistanceScale_ * distance;

    // Compute the icon size and margin.
    const float icon_size = distance_scale * Defaults::kIconSize;
    const float margin    = distance_scale * kMargin_;

    // Compute the shelf width based on the number of icons.
    const size_t icon_count = icons.size();
    const float shelf_width =
        icon_count * icon_size + (icon_count + 1) * margin;

    // Position in X based on width and in Y based on height (plus a little).
    float x = -.5f * shelf_width + margin + .5f * icon_size;
    const float y = .5f * icon_size + kExtraHeight_;

    // Scale each icon to the correct size, position it, and enable it.
    for (auto &icon: icons) {
        auto &widget = icon->GetWidget();
        ASSERT(widget);
        const Bounds bounds = widget->GetBounds();
        const int max_index = GetMaxElementIndex(bounds.GetSize());
        const float scale = icon_size / bounds.GetSize()[max_index];
        widget->SetScale(scale * widget->GetScale());

        // Position in Z so that the front of the icon is just past the front
        // of the shelf.
        const float z = 2.f + .5f * (shelf_depth - scale * bounds.GetSize()[2]);

        widget->SetTranslation(Vector3f(x, y, z));
        x += icon_size + margin;

        // Enable the widget and add it as a child of the parent.
        widget->SetEnabled(SG::Node::Flag::kTraversal, true);
        parent.AddChild(widget);
    }

    return shelf_width;
}
