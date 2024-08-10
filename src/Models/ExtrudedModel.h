#pragma once

#include "Math/Profile.h"
#include "Math/Types.h"
#include "Models/PrimitiveModel.h"
#include "Util/Memory.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(ExtrudedModel);

/// ExtrudedModel is a derived Model that represents a linear extrusion of a
/// planar polygon in the Y=0 plane along the +Y axis.
///
/// \ingroup Models
class ExtrudedModel : public PrimitiveModel {
  public:
    /// Sets the Profile to use for the surface. Asserts if it is not a valid
    /// closed Profile.
    void SetProfile(const Profile &profile);

    /// Returns the current profile.
    const Profile & GetProfile() const { return profile_; }

    /// Convenience that creates an ExtrudedModel Profile that is a regular
    /// polygon with the given number of sides (>= 3). Asserts on error.
    static Profile CreateRegularPolygonProfile(size_t side_count);

    /// Convenience that creates a valid ExtrudedModel Profile from the given
    /// vector of movable profile points. Asserts if there are fewer than 3
    /// points.
    static Profile CreateProfile(const Profile::PointVec &points);

    /// Convenience that creates and returns a default Profile for an
    /// ExtrudedModel. It is a square.
    static Profile CreateDefaultProfile();

  protected:
    ExtrudedModel() {}
    virtual void AddFields() override;
    virtual bool IsValid(Str &details) override;
    virtual void CreationDone() override;
    virtual TriMesh BuildMesh() override;

  private:
    /// \name Parsed fields.
    ///@{
    Parser::VField<Point2f> profile_points_;
    ///@}

    /// Stores the current Profile.
    Profile profile_{ CreateDefaultProfile() };

    friend class Parser::Registry;
};
