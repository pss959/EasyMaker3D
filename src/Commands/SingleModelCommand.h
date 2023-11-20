#pragma once

#include "Commands/Command.h"

/// SingleModelCommand is an abstract base class for command classes that
/// operate on exactly one selected Model. It stores the names of the Model.
///
/// \ingroup Commands
class SingleModelCommand : public Command {
  public:
    /// Sets the Model name from the Model in the given selection, which must
    /// contain exactly one Model.
    void SetFromSelection(const Selection &sel);

    /// Returns the name of the operand Model.
    const Str & GetModelName() const { return model_name_; }

  protected:
    virtual void AddFields() override;
    virtual bool IsValid(Str &details) override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<Str> model_name_;
    ///@}
};
