//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include <vector>

#include "SG/Node.h"
#include "Util/Memory.h"
#include "Widgets/IconWidget.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(Shelf);

/// The Shelf class is used for setting up a shelf in the scene that has
/// interactive 3D icon buttons on it. It handles resizing of the shelf
/// geometry to fit all added icons.
///
/// \ingroup Items
class Shelf : public SG::Node {
  public:
    /// Returns the IconWidget instances for the Shelf.
    const std::vector<IconWidgetPtr> & GetIcons() const { return icons_; }

    /// Lays out all icons on the Shelf. The position of the main camera is
    /// supplied to scale icons so the far ones are not too small.
    void LayOutIcons(const Point3f &cam_pos);

  protected:
    Shelf() {}

    virtual void AddFields() override;
    virtual void CreationDone() override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<float>               depth_scale_;
    Parser::ObjectListField<IconWidget> icons_;
    ///@}

    /// Node representing the shelf geometry.
    SG::NodePtr node_;

    /// Adds the icons to the shelf. Returns the correct shelf width.
    float AddIcons_(float distance, float shelf_depth);

    friend class Parser::Registry;
};
