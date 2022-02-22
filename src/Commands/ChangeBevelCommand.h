#pragma once

#include <memory>

#include "Commands/MultiModelCommand.h"
#include "Defaults.h"
#include "Math/Bevel.h"

/// ChangeBevelCommand is used to change the Bevel data in one or more
/// BeveledModel instances.
///
/// \ingroup Commands
class ChangeBevelCommand : public MultiModelCommand {
  public:
    virtual std::string GetDescription() const override;

    /// Sets a new set of Profile points for the Bevel. Note that these do not
    /// include fixed points.
    void SetProfilePoints(const std::vector<Point2f> &points) {
        profile_points_ = points;
    }

    /// Sets the new scale factor for the Bevel.
    void SetBevelScale(float scale) { bevel_scale_ = scale; }

    /// Sets the new maximum angle for the Bevel.
    void SetMaxAngle(const Anglef &angle) { max_angle_ = angle; }

    /// Returns a Bevel representing the new Bevel data in the command.
    Bevel GetBevel() const;

  protected:
    ChangeBevelCommand() {}

    virtual void AddFields() override;
    virtual bool IsValid(std::string &details) override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::VField<Point2f> profile_points_{"profile_points"};
    Parser::TField<float>   bevel_scale_{"bevel_scale", 1};
    Parser::TField<Anglef>  max_angle_{
        "max_angle", {Anglef::FromDegrees(Defaults::kMaxBevelAngle)}};
    ///@}

    friend class Parser::Registry;
};

typedef std::shared_ptr<ChangeBevelCommand> ChangeBevelCommandPtr;
