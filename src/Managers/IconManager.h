#pragma once

#include <vector>

#include "SG/Typedefs.h"
#include "Widgets/Widget.h"

/// The IconManager class manages the placement and enabling of all interactive
/// 3D Icon buttons in the scene. Each is either a PushButtonWidget or a
/// ToggleButtonWidget.
///
/// Icons are organized into and laid out in shelves. Shelf width is adjusted to
/// fit all of the icons on it.
///
/// The ProcessUpdate() function should be called every frame to update the
/// enabled/disabled state of all managed icons.
///
/// \ingroup Managers
class IconManager {
  public:
    /// Sets up a shelf with the given icon widgets.
    void AddShelf(SG::Node &shelf, const std::vector<WidgetPtr> &widgets);
};
