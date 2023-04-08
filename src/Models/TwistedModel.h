#pragma once

#include "Base/Memory.h"
#include "Math/Types.h"
#include "Models/ConvertedModel.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(TwistedModel);

/// TwistedModel is a derived ConvertedModel class that represents a Model that
/// has been twisted by some angle around an arbitrary axis through an
/// arbitrary point. The point and axis are specified in object coordinates of
/// the TwistedModel.
///
/// \ingroup Models
class TwistedModel : public ConvertedModel {
  public:
    /// Struct defining a twist.
    struct Twist {
        Point3f  center;  ///< Twist center in object coordinates.
        Vector3f axis;    ///< Twist axis in object coordinates.
        Anglef   angle;   ///< Twist angle.
    };

    /// Sets the twist parameters.
    void SetTwist(const Twist &twist);

    /// Returns the twist parameters.
    Twist GetTwist() const;

  protected:
    TwistedModel() {}
    virtual void AddFields() override;
    virtual TriMesh BuildMesh() override;

  private:
    /// \name Parsed fields.
    ///@{
    Parser::TField<Point3f>  center_;
    Parser::TField<Vector3f> axis_;
    Parser::TField<Anglef>   angle_;
    ///@}

    friend class Parser::Registry;
};
