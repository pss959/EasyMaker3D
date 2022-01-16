﻿#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Commands/Command.h"
#include "Util/General.h"

/// MultiModelCommand is an abstract base class for command classes that
/// operate on multiple Models. It stores the names of all of the Models.
///
/// \ingroup Commands
class MultiModelCommand : public Command {
  public:
    /// Sets the Model names from all Models in the given selection.
    void SetFromSelection(const Selection &sel);

    /// Returns the names of the operand Models.
    const std::vector<std::string> & GetModelNames() const {
        return model_names_;
    }

  protected:
    virtual void AddFields() override;
    virtual bool IsValid(std::string &details) override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::VField<std::string> model_names_{"model_names"};
    ///@}
};
