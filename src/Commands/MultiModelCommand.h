#pragma once

#include <string>
#include <vector>

#include "Commands/Command.h"

/// MultiModelCommand is an abstract base class for command classes that
/// operate on multiple Models. It stores the names of all of the Models.
///
/// \ingroup Commands
class MultiModelCommand : public Command {
  public:
    /// Sets the Model names.
    void SetModelNames(const StrVec &names) { model_names_ = names; }

    /// Sets the Model names from all Models in the given selection.
    void SetFromSelection(const Selection &sel);

    /// Returns the names of the operand Models.
    const StrVec & GetModelNames() const { return model_names_; }

  protected:
    virtual void AddFields() override;
    virtual bool IsValid(Str &details) override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::VField<Str> model_names_;
    ///@}
};
