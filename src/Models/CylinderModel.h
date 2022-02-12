#pragma once

#include "Models/PrimitiveModel.h"

namespace Parser { class Registry; }

/// CylinderModel is a derived PrimitiveModel class representing a cylinder. It
/// has independent top and bottom radii. By default, both radii are 1 and Y
/// (the height dimension) ranges from -1 to 1.
///
/// A complexity 0 cylinder has 3 sides, while a complexity 1 cylinder has 120
/// sides.
///
/// \ingroup Models
class CylinderModel : public PrimitiveModel {
  public:
    /// Enum indicating one of the two radii.
    enum class Radius { kTop, kBottom };

    /// Sets the top or bottom radius, clamped to be at least kMinRadius.
    void SetRadius(Radius which, float radius);

    /// Returns the current top or bottom radius value.
    float GetRadius(Radius which) const {
        return which == Radius::kTop ? top_radius_ : bottom_radius_;
    }

    /// CylinderModel responds to complexity.
    virtual bool CanSetComplexity() const override { return true; }

    /// Minimum radius value for top or bottom radius.
    static constexpr float kMinRadius = .01f;

  protected:
    CylinderModel() {}
    virtual void AddFields() override;
    virtual bool IsValid(std::string &details) override;
    virtual TriMesh BuildMesh() override;
    virtual void CreationDone() override;

  private:
    /// \name Parsed fields.
    ///@{
    Parser::TField<float> top_radius_{"top_radius", 1};
    Parser::TField<float> bottom_radius_{"bottom_radius", 1};
    ///@}

    friend class Parser::Registry;
};

typedef std::shared_ptr<CylinderModel> CylinderModelPtr;
