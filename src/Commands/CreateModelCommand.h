#pragma once

#include <string>

#include "Base/Memory.h"
#include "Commands/Command.h"
#include "Math/Types.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(CreateModelCommand);

/// CreateModelCommand is an abstract base class for Commands used to create a
/// Model of some sort.
///
/// \ingroup Commands
class CreateModelCommand : public Command {
  public:
    /// Returns the name of the resulting Model. This will be empty unless this
    /// Command was read from a file and a name was specified.
    const std::string & GetResultName() const { return result_name_; }

    /// Sets the name of the resulting Model.
    void SetResultName(const std::string &name);

    /// Returns the target position (in stage coordinates) for the new
    /// Model. This is the origin by default unless the PointTarget is active.
    const Point3f & GetTargetPosition() const { return target_position_; }

    /// Sets the target position (in stage coordinates) for the new Model. This
    /// is the origin by default.
    void SetTargetPosition(const Point3f &p) { target_position_ = p; }

    /// Returns the target direction (in stage coordinates) for the new
    /// Model. This is the +Y axis by default unless the PointTarget is active.
    const Vector3f & GetTargetDirection() const { return target_direction_; }

    /// Sets the target direction (in stage coordinates) for the new
    /// Model. This is the +Y axis by default.
    void SetTargetDirection(const Vector3f &d) { target_direction_ = d; }

  protected:
    CreateModelCommand() {}

    virtual void AddFields() override;
    virtual bool IsValid(std::string &details) override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<std::string> result_name_{"result_name"};
    Parser::TField<Point3f>     target_position_{"target_position",   {0,0,0}};
    Parser::TField<Vector3f>    target_direction_{"target_direction", {0,1,0}};
    ///@}

    friend class Parser::Registry;
};
