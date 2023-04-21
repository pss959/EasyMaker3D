#pragma once

#include "Base/Memory.h"
#include "Commands/MultiModelCommand.h"
#include "Math/Types.h"

struct Twist;
namespace Parser { class Registry; }

DECL_SHARED_PTR(ChangeTwistCommand);

/// ChangeTwistCommand is used to change the twist applied one or more
/// TwistedModel instances.
///
/// \ingroup Commands
class ChangeTwistCommand : public MultiModelCommand {
  public:
    virtual std::string GetDescription() const override;

    /// Sets the twist parameters to those in the given Twist instance.
    void SetTwist(const Twist &twist);

    Twist GetTwist() const;

  protected:
    ChangeTwistCommand() {}

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
