#pragma once

#include "Commands/MultiModelCommand.h"
#include "Math/Spin.h"
#include "Math/Types.h"
#include "Util/Memory.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(ChangeSpinCommand);

/// ChangeSpinCommand is a base class for commands used to modify a Spin that
/// represents a rotation of some sort around an axis in one or more Models.
///
/// \ingroup Commands
class ChangeSpinCommand : public MultiModelCommand {
  public:
    /// Sets the parameters based on the given Spin.
    void SetSpin(const Spin &spin);

    /// Returns the values as an Spin struct.
    Spin GetSpin() const;

  protected:
    ChangeSpinCommand() {}

    virtual void AddFields() override;
    virtual bool IsValid(Str &details) override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<Point3f>  center_;
    Parser::TField<Vector3f> axis_;
    Parser::TField<Anglef>   angle_;
    Parser::TField<float>    offset_;
    ///@}

    friend class Parser::Registry;
};
