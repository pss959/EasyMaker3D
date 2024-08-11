//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include "Commands/MultiModelCommand.h"
#include "Util/Memory.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(CombineCommand);

/// CombineCommand is an abstract base class for command classes that create a
/// CombinedModel of some sort from one or more operand Models. It exists
/// solely for type information.
///
/// \ingroup Commands
class CombineCommand : public MultiModelCommand {
  public:
    /// Returns the name of the resulting CombinedModel. This will be empty
    /// unless this Command was read from a file and a name was specified.
    const Str & GetResultName() const { return result_name_; }

    /// Sets the name of the resulting CombinedModel.
    void SetResultName(const Str &name);

  protected:
    virtual void AddFields() override;
    virtual bool IsValid(Str &details) override;

    /// Useful convenience for building description strings.
    Str BuildDescription(const Str &type) const;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<Str> result_name_;
    ///@}

    friend class Parser::Registry;
};
