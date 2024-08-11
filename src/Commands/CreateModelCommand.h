//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include "Commands/Command.h"
#include "Math/Types.h"
#include "Util/Memory.h"

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
    const Str & GetResultName() const { return result_name_; }

    /// Sets the name of the resulting Model.
    void SetResultName(const Str &name);

    /// Returns the initial (uniform) scale applied to the new Model. This is 1
    /// by default.
    float GetInitialScale() const { return initial_scale_; }

    /// Sets the initial (uniform) scale for the new Model. This is 1 by
    /// default.
    void SetInitialScale(float scale) { initial_scale_ = scale; }

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
    virtual bool IsValid(Str &details) override;

    /// Useful convenience for building description strings.
    Str BuildDescription(const Str &type) const;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<Str>         result_name_;
    Parser::TField<float>       initial_scale_;
    Parser::TField<Point3f>     target_position_;
    Parser::TField<Vector3f>    target_direction_;
    ///@}

    friend class Parser::Registry;
};
