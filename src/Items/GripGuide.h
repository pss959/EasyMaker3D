//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include <vector>

#include "Enums/GripGuideType.h"
#include "Math/Types.h"
#include "SG/Node.h"
#include "Util/Memory.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(GripGuide);

/// A GripGuide represents geometry that is part of a Controller to help
/// indicate what operation can be performed. An active Grippable indicates
/// which type of guide should be displayed. Hover highlighting emanates from a
/// special point on the GripGuide to an interactive Widget.
///
/// \ingroup Items
class GripGuide : public SG::Node {
  public:
    /// Returns the GripGuideType associated with this GripGuide.
    GripGuideType GetGripGuideType() const { return guide_type_; }

    /// Returns the hover attachment point for the current guide relative to
    /// the GripGuide.
    const Point3f & GetHoverPoint() const { return hover_point_; }

  protected:
    GripGuide() {}

    virtual void AddFields() override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::EnumField<GripGuideType> guide_type_;
    Parser::TField<Point3f>          hover_point_;
    ///@}

    friend class Parser::Registry;
};
