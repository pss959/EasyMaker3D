#pragma once

#include "Commands/MultiModelCommand.h"
#include "Defaults.h"
#include "Math/Bevel.h"
#include "Memory.h"

DECL_SHARED_PTR(ChangeBevelCommand);

/// ChangeBevelCommand is used to change the Bevel data in one or more
/// BeveledModel instances.
///
/// \ingroup Commands
class ChangeBevelCommand : public MultiModelCommand {
  public:
    virtual std::string GetDescription() const override;

    /// Sets all the fields from the given Bevel.
    void SetBevel(const Bevel &bevel);

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
