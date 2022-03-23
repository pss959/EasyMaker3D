#pragma once

#include <memory>
#include <string>

#include "Commands/Command.h"

/// CreateModelCommand is an abstract base class for Commands used to create a
/// Model of some sort.
///
/// \ingroup Commands
class CreateModelCommand : public Command {
  public:
    /// Returns the name of the resulting Model. This will be empty unless this
    /// Command was read from a file and a name was specified.
    const std::string & GetResultName() const { return result_name_; }

    /// Sets the name of the resulting Model.
    void SetResultName(const std::string &name);

  protected:
    CreateModelCommand() {}

    virtual void AddFields() override;
    virtual bool IsValid(std::string &details) override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<std::string> result_name_{"result_name"};
    ///@}

    friend class Parser::Registry;
};

typedef std::shared_ptr<CreateModelCommand> CreateModelCommandPtr;
