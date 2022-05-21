#pragma once

#include "Base/Memory.h"
#include "Commands/MultiModelCommand.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(CombineCommand);

/// CombineCommand is an abstract base class for command classes that create a
/// CombinedModel of some sort from one or more operand Models. It exists
/// solely for type information.
///
/// \ingroup Commands
class CombineCommand : public MultiModelCommand {
  public:
    /// Returns the name of the resulting CombinedModel. This will be empty
    /// unless this Command was read from a file and a name was specified.
    const std::string & GetResultName() const { return result_name_; }

    /// Sets the name of the resulting CombinedModel.
    void SetResultName(const std::string &name);

  protected:
    virtual void AddFields() override;
    virtual bool IsValid(std::string &details) override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<std::string> result_name_{"result_name"};
    ///@}

    friend class Parser::Registry;
};
