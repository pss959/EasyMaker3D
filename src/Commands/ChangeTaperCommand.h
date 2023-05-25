#pragma once

#include "Base/Memory.h"
#include "Commands/MultiModelCommand.h"
#include "Enums/Dim.h"
#include "Math/Taper.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(ChangeTaperCommand);

/// ChangeTaperCommand is used to change the taper profile in one or more
/// TaperedModel instances.
///
/// \ingroup Commands
class ChangeTaperCommand : public MultiModelCommand {
  public:
    virtual std::string GetDescription() const override;

    /// Sets all the fields from the given Taper.
    void SetTaper(const Taper &taper);

    /// Returns a Taper representing the new Taper data in the command.
    Taper GetTaper() const;

  protected:
    ChangeTaperCommand() {}

    virtual void AddFields() override;
    virtual bool IsValid(std::string &details) override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::EnumField<Dim>  axis_;
    Parser::VField<Point2f> profile_points_;
    ///@}

    friend class Parser::Registry;
};
