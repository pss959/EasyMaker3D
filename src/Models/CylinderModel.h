#include "Models/PrimitiveModel.h"

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
    virtual void AddFields() override;

    /// Redefines this to check for errors.
    virtual void AllFieldsParsed() override;

    /// Sets the top radius, clamped to be at least kMinRadius.
    void SetTopRadius(float radius);

    /// Sets the bottom radius, clamped to be at least kMinRadius.
    void SetBottomRadius(float radius);

    /// Returns the current top radius.
    float GetTopRadius() const { return top_radius_; }

    /// Returns the current bottom radius.
    float GetBottomRadius() const { return bottom_radius_; }

    /// CylinderModel responds to complexity.
    virtual bool CanSetComplexity() const override { return true; }

    /// Minimum radius value for top or bottom radius.
    static constexpr float kMinRadius = .01f;

  protected:
    virtual TriMesh BuildMesh() override;

  private:
    /// \name Parsed fields.
    ///@{
    Parser::TField<float> top_radius_{"top_radius", 1};
    Parser::TField<float> bottom_radius_{"bottom_radius", 1};
    ///@}
};

typedef std::shared_ptr<CylinderModel> CylinderModelPtr;
