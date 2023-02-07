#pragma once

#include "Base/Memory.h"
#include "Math/Types.h"
#include "Parser/Object.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(EdgeTarget);

/// EdgeTarget represents the edge-based target for interactive snapping
/// operations. It contains the position of both ends of the edge.
///
/// \ingroup Place
class EdgeTarget : public Parser::Object {
  public:
    /// Sets the position of the start and end of the edge in stage coordinates.
    void SetPositions(const Point3f &pos0, const Point3f &pos1) {
        position0_ = pos0;
        position1_ = pos1;
    }

    /// Returns the position of the start of the edge in stage coordinates.
    const Point3f & GetPosition0() const { return position0_; }

    /// Returns the position of the end of the edge in stage coordinates.
    const Point3f & GetPosition1() const { return position1_; }

    /// Returns the normalized direction vector for the edge.
    Vector3f GetDirection() const;

    //! Returns the length of the edge.
    float GetLength() const;

    /// Copies values from another instance.
    void CopyFrom(const EdgeTarget &from) { CopyContentsFrom(from, false); }

  protected:
    EdgeTarget() {}

    virtual void AddFields() override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<Point3f> position0_;
    Parser::TField<Point3f> position1_;
    ///@}

    friend class Parser::Registry;
};
