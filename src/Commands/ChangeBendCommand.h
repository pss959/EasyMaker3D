#pragma once

#include "Base/Memory.h"
#include "Commands/MultiModelCommand.h"
#include "Math/Types.h"

struct Bend;
namespace Parser { class Registry; }

DECL_SHARED_PTR(ChangeBendCommand);

/// ChangeBendCommand is used to change the bend applied one or more
/// BentModel instances.
///
/// \ingroup Commands
class ChangeBendCommand : public MultiModelCommand {
  public:
    virtual std::string GetDescription() const override;

    /// Sets the bend parameters to those in the given Bend instance.
    void SetBend(const Bend &bend);

    Bend GetBend() const;

  protected:
    ChangeBendCommand() {}

    virtual void AddFields() override;
    virtual bool IsValid(std::string &details) override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<Point3f>  center_;
    Parser::TField<Vector3f> axis_;
    Parser::TField<Anglef>   angle_;
    ///@}

    friend class Parser::Registry;
};
