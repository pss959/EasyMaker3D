#pragma once

#include "Base/Memory.h"
#include "Commands/SingleModelCommand.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(ChangeNameCommand);

/// ChangeNameCommand is used to change the name of the currently selected
/// Model.
///
/// \ingroup Commands
class ChangeNameCommand : public SingleModelCommand {
  public:
    virtual std::string GetDescription() const override;

    /// Sets the new name.
    void SetNewName(const std::string &new_name);

    /// Returns the new name.
    const std::string & GetNewName() const { return new_name_; }

  protected:
    ChangeNameCommand() {}

    virtual void AddFields() override;
    virtual bool IsValid(std::string &details) override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<std::string> new_name_{"new_name"};
    ///@}

    friend class Parser::Registry;
};
