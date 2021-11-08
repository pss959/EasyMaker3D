#pragma once

#include <memory>
#include <vector>

#include "Enums/GripGuideType.h"
#include "Math/Types.h"
#include "SG/Node.h"

namespace Parser { class Registry; }

/// A GripGuide is attached to each Controller object to help indicate what
/// operation can be performed. An active Grippable indicates which type of
/// guide should be displayed. Hover highlighting emanates from a special point
/// on the GripGuide to an interactive Widget.
///
/// Each type of guide is a child of the GripGuide; at most one is made visible
/// at any time. The relative positions of guides indicate how they are
/// attached to a Hand. The relative location of the "HoverPoint" node in each
/// child guide indicates where the hover highlight should emanate from.
///
/// The hover guides are designed to have the hand attachment part at the
/// origin and be roughly 5-10 units in length.
class GripGuide : public SG::Node {
  public:
    /// Sets the type of guide geometry to display. It is GripGuideType::kNone
    /// by default.
    void SetGuideType(GripGuideType type);

    /// Returns the current guide type.
    GripGuideType GetGuideType() const { return guide_type_; }

    /// Returns the hover attachment point for the current guide relative to
    /// the GripGuide. Returns the zero vector if the current guide is
    /// GripGuideType::kNone.
    Point3f GetHoverPoint() const;

    virtual void PostSetUpIon() override;

  protected:
    GripGuide() {}

  private:
    /// Current Guide displayed.
    GripGuideType guide_type_ = GripGuideType::kNone;

    /// Stores the relative location of each hover point for each type.
    std::vector<Point3f> hover_points_;

    friend class Parser::Registry;
};

typedef std::shared_ptr<GripGuide> GripGuidePtr;
