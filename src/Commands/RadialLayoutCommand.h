#pragma once

#include "Base/Memory.h"
#include "Commands/MultiModelCommand.h"
#include "Math/Curves.h"
#include "Math/Types.h"

namespace Parser { class Registry; }

class PointTarget;
DECL_SHARED_PTR(RadialLayoutCommand);

/// RadialLayoutCommand is used to lay out the currently selected Models
/// in a radial pattern in a plane.
///
/// \ingroup Commands
class RadialLayoutCommand : public MultiModelCommand {
  public:
    virtual std::string GetDescription() const override;

    /// Sets the fields in the command from a PointTarget instance.
    void SetFromTarget(const PointTarget &target);

    /// Returns the center point.
    const Point3f & GetCenter() const { return center_; }

    /// Returns the normal to the layout plane.
    const Vector3f & GetNormal() const { return normal_; }

    /// Returns the layout radius.
    float GetRadius() const { return radius_; }

    /// Returns the layout arc.
    const CircleArc & GetArc() const { return arc_; }

  protected:
    RadialLayoutCommand() {}

    virtual void AddFields() override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<Point3f>   center_{"center", {0, 0, 0}};
    Parser::TField<Vector3f>  normal_{"normal", {0, 1, 0}};
    Parser::TField<float>     radius_{"radius", 1};
    Parser::TField<CircleArc> arc_{"arc"};
    ///@}

    friend class Parser::Registry;
};
