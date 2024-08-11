//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include "Commands/MultiModelCommand.h"
#include "Util/Memory.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(RotateCommand);

/// RotateCommand is used to rotate the currently selected Models. Scaling can be
/// be in place (each Model rotating about its center) or not (all Models rotate
/// about the primary Model's center).
///
/// \ingroup Commands
class RotateCommand : public MultiModelCommand {
  public:
    virtual Str GetDescription() const override;

    /// Returns the rotation being applied, which is in stage coordinates if
    /// IsAxisAligned() is true or in the local coordinate of the primary Model
    /// otherwise.
    const Rotationf & GetRotation() const { return rotation_; }

    /// Sets the rotation to apply.
    void SetRotation(const Rotationf &rot) { rotation_ = rot; }

    /// Returns true if rotating all Models in place. The default is false.
    bool IsInPlace() const { return is_in_place_; }

    /// Sets whether to rotate all Models in place or to rotate all Models
    /// around the center of the primary Model. The default is false.
    void SetIsInPlace(bool is_in_place) { is_in_place_ = is_in_place; }

    /// Returns true if rotation is aligned with stage coordinate axes rather
    /// than the Model's object coordinates. The default is false.
    bool IsAxisAligned() const { return is_axis_aligned_; }

    /// Sets whether rotation is aligned with stage coordinates rather than the
    /// Model's object coordinates. The default is false.
    void SetIsAxisAligned(bool aligned) { is_axis_aligned_ = aligned; }

  protected:
    RotateCommand() {}

    virtual void AddFields() override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<Rotationf> rotation_;
    Parser::TField<bool>      is_in_place_;
    Parser::TField<bool>      is_axis_aligned_;
    ///@}

    friend class Parser::Registry;
};
