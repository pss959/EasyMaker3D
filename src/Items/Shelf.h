#pragma once

#include <memory>

#include "SG/Node.h"
#include "Widgets/Widget.h"

namespace Parser { class Registry; }

/// The Shelf class is used for setting up a shelf in the scene that has
/// interactive 3D icon buttons on it. It handles resizing of the shelf
/// geometry to fit all added icons.
class Shelf : public SG::Node {
  public:
    /// Initializes the Shelf with the Node that represents the shelf geometry,
    /// the 3D icons (as Widgets) that will appear on the shelf, and the
    /// distance of the shelf from the main camera (which is used to scale
    /// icons so the far ones are not too small).
    void Init(const SG::NodePtr &shelf_geometry,
              const std::vector<WidgetPtr> &icons, float distance);

  protected:
    Shelf() {}

  private:
    /// Node representing the shelf geometry.
    SG::NodePtr node_;

    /// Adds the icons to the shelf. Returns the correct shelf width.
    float AddIcons_(Node &parent, const std::vector<WidgetPtr> &icons,
                    float distance);

    friend class Parser::Registry;
};

typedef std::shared_ptr<Shelf> ShelfPtr;
