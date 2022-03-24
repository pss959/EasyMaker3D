#pragma once

#include <memory>

#include "Commands/MultiModelCommand.h"

/// ChangeComplexityCommand is used to change the complexity of the currently
/// selected Models.
///
/// \ingroup Commands
class ChangeComplexityCommand : public MultiModelCommand {
  public:
    virtual std::string GetDescription() const override;

    /// Sets the new complexity.
    void SetNewComplexity(float new_complexity) {
        new_complexity_ = new_complexity;
    }

    /// Returns the new complexity.
    float GetNewComplexity() const { return new_complexity_; }

  protected:
    ChangeComplexityCommand() {}

    virtual void AddFields() override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<float> new_complexity_{"new_complexity"};
    ///@}

    friend class Parser::Registry;
};

typedef std::shared_ptr<ChangeComplexityCommand> ChangeComplexityCommandPtr;
