#include "Math/Bevel.h"
#include "Models/ConvertedModel.h"

namespace Parser { class Registry; }

/// BeveledModel is a derived ConvertedModel class that represents a Model
/// whose edges have had a bevel or rounding operation applied to them.
///
/// \ingroup Models
class BeveledModel : public ConvertedModel {
  public:
    virtual void AddFields() override;

    virtual bool IsValid(std::string &details) override;

    /// Sets the Bevel to use.
    void SetBevel(const Bevel &bevel);

    /// Returns the current Bevel.
    const Bevel & GetBevel() const { return bevel_; }

  protected:
    BeveledModel() {}
    virtual TriMesh BuildMesh() override;

  private:
    /// \name Parsed fields.
    ///@{
    Parser::VField<Point2f> profile_points_{"profile_points"};
    Parser::TField<float>   bevel_scale_{"bevel_scale", 1};
    Parser::TField<Anglef>  max_angle_{"max_angle", {GetDefaultMaxAngle()}};
    ///@}

    /// Bevel used to create the model.
    Bevel bevel_;

    /// Returns the default max angle setting.
    static Anglef GetDefaultMaxAngle() {
        return Anglef::FromDegrees(Defaults::kMaxBevelAngle);
    }

    friend class Parser::Registry;
};

typedef std::shared_ptr<BeveledModel> BeveledModelPtr;
