#pragma once

#include "Base/Memory.h"
#include "Commands/MultiModelCommand.h"
#include "Math/Bevel.h"  // XXXX

namespace Parser { class Registry; }

DECL_SHARED_PTR(ChangeTaperCommand);

/// ChangeTaperCommand is used to change the Taper data in one or more
/// TaperedModel instances.
///
/// \ingroup Commands
class ChangeTaperCommand : public MultiModelCommand {
  public:
    virtual std::string GetDescription() const override;

    /// Sets all the fields from the given Taper.
    void SetTaper(const Bevel &taper);

    /// Returns a Taper representing the new Taper data in the command.
    Bevel GetTaper() const;

  protected:
    ChangeTaperCommand() {}

    virtual void AddFields() override;
    virtual bool IsValid(std::string &details) override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::VField<Point2f> profile_points_;
    Parser::TField<float>   taper_scale_;
    Parser::TField<Anglef>  max_angle_;
    ///@}

    friend class Parser::Registry;
};
