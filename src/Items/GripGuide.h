#pragma once

#include <vector>

#include "Base/Memory.h"
#include "Enums/GripGuideType.h"
#include "Math/Types.h"
#include "SG/Node.h"

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
    Parser::EnumField<GripGuideType> guide_type_{"guide_type"};
    Parser::TField<Point3f>          hover_point_{"hover_point", {0, 0, 0}};
    ///@}

    friend class Parser::Registry;
};
