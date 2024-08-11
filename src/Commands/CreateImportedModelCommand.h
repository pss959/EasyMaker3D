//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include "Commands/CreateModelCommand.h"
#include "Util/Memory.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(CreateImportedModelCommand);

/// CreateImportedModelCommand is used to create an ImportedModel.
///
/// \ingroup Commands
class CreateImportedModelCommand : public CreateModelCommand {
  public:
    virtual Str GetDescription() const override;

    /// Sets the path for the ImportedModel as a string.
    void SetPath(const Str &path) { path_ = FixPath(path); }

    /// Returns the path for the ImportedModel as a string.
    const Str & GetPath() const { return path_; }

  protected:
    CreateImportedModelCommand() {}

    virtual void AddFields() override;
    virtual void CreationDone() override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<Str> path_;
    ///@}

    friend class Parser::Registry;
};
