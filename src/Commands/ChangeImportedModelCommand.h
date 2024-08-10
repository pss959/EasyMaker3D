#pragma once

#include "Commands/SingleModelCommand.h"
#include "Util/Memory.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(ChangeImportedModelCommand);

/// ChangeImportedModelCommand is used to change the import path for an
/// ImportedModel.
///
/// \ingroup Commands
class ChangeImportedModelCommand : public SingleModelCommand {
  public:
    virtual Str GetDescription() const override;

    /// Sets the new path for the ImportedModel as a string.
    void SetNewPath(const Str &path) { new_path_ = FixPath(path); }

    /// Returns the new path for the ImportedModel as a string.
    const Str & GetNewPath() const { return new_path_; }

  protected:
    ChangeImportedModelCommand() {}

    virtual void AddFields() override;
    virtual bool IsValid(Str &details) override;
    virtual void CreationDone() override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<Str> new_path_;
    ///@}

    friend class Parser::Registry;
};
