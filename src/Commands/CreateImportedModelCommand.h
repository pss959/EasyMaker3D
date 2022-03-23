﻿#pragma once

#include <memory>
#include <string>

#include "Commands/CreateModelCommand.h"

/// CreateImportedModelCommand is used to create an ImportedModel.
///
/// \ingroup Commands
class CreateImportedModelCommand : public CreateModelCommand {
  public:
    virtual std::string GetDescription() const override;

    /// Sets the path for the ImportedModel as a string.
    void SetPath(const std::string &path) { path_ = path; }

    /// Returns the path for the ImportedModel as a string.
    const std::string & GetPath() const { return path_; }

  protected:
    CreateImportedModelCommand() {}

    virtual void AddFields() override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<std::string> path_{"path"};
    ///@}

    friend class Parser::Registry;
};

typedef std::shared_ptr<CreateImportedModelCommand>
    CreateImportedModelCommandPtr;
