#pragma once

#include "Base/Memory.h"
#include "Commands/SingleModelCommand.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(ChangeImportedModelCommand);

/// ChangeImportedModelCommand is used to change the import path for an
/// ImportedModel.
///
/// \ingroup Commands
class ChangeImportedModelCommand : public SingleModelCommand {
  public:
    virtual std::string GetDescription() const override;

    /// Sets the new path for the ImportedModel as a string.
    void SetNewPath(const std::string &path) { new_path_ = FixPath(path); }

    /// Returns the new path for the ImportedModel as a string.
    const std::string & GetNewPath() const { return new_path_; }

  protected:
    ChangeImportedModelCommand() {}

    virtual void AddFields() override;
    virtual bool IsValid(std::string &details) override;
    virtual void CreationDone() override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<std::string> new_path_;
    ///@}

    friend class Parser::Registry;
};
