#pragma once

#include <memory>

#include "Commands/SingleModelCommand.h"

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

typedef std::shared_ptr<ChangeNameCommand> ChangeNameCommandPtr;
