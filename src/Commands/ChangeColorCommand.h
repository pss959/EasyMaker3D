//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include "Commands/MultiModelCommand.h"
#include "Math/Color.h"
#include "Math/Types.h"
#include "Util/Memory.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(ChangeColorCommand);

/// ChangeColorCommand is used to change the color of the currently selected
/// Models.
///
/// \ingroup Commands
class ChangeColorCommand : public MultiModelCommand {
  public:
    virtual Str GetDescription() const override;

    /// Sets the new color.
    void SetNewColor(const Color &new_color) { new_color_ = new_color; }

    /// Returns the new color.
    const Color & GetNewColor() const { return new_color_; }

  protected:
    ChangeColorCommand() {}

    virtual void AddFields() override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<Color> new_color_;
    ///@}

    friend class Parser::Registry;
};
