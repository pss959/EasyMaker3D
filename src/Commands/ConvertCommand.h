#pragma once

#include "Base/Memory.h"
#include "Commands/MultiModelCommand.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(ConvertCommand);

/// ConvertCommand is an abstract base class for command classes that create a
/// ConvertedModel of some sort from one or more operand Models.
///
/// \ingroup Commands
class ConvertCommand : public MultiModelCommand {
  public:
    /// Sets the new names of the resulting ConvertedModels.
    void SetResultNames(const std::vector<std::string> &names) {
        result_names_ = names;
    }

    /// Returns the new names of the resulting ConvertedModels.
    const std::vector<std::string> & GetResultNames() const {
        return result_names_;
    }

  protected:
    virtual void AddFields() override;
    virtual bool IsValid(std::string &details) override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::VField<std::string>     result_names_;
    ///@}

    friend class Parser::Registry;
};
