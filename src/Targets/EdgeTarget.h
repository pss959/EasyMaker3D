#pragma once

#include <memory>

#include "Math/Types.h"
#include "Parser/Object.h"

namespace Parser { class Registry; }

/// EdgeTarget represents the edge-based target for interactive snapping
/// operations. It contains the position of both ends of the edge.
///
/// \ingroup Targets
class EdgeTarget : public Parser::Object {
  public:
    /// Returns the position of the start of the edge in stage coordinates.
    const Point3f & GetPosition0() const { return position0_; }

    /// Returns the position of the end of the edge in stage coordinates.
    const Point3f & GetPosition1() const { return position1_; }

    /// Returns the normalized direction vector for the edge.
    Vector3f GetDirection() const;

    //! Returns the length of the edge.
    float GetLength() const;

  protected:
    EdgeTarget() {}

    virtual void AddFields() override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<Point3f> position0_{"position0", {0,  0, 0}};
    Parser::TField<Point3f> position1_{"position0", {0, 10, 0}};
    ///@}

    friend class Parser::Registry;
};

typedef std::shared_ptr<EdgeTarget> EdgeTargetPtr;
