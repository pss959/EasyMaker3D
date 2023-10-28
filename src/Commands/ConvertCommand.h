#pragma once

#include "Commands/MultiModelCommand.h"
#include "Util/Memory.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(ConvertCommand);

/// ConvertCommand is an abstract base class for command classes that create a
/// ConvertedModel of some sort from one or more operand Models.
///
/// \ingroup Commands
class ConvertCommand : public MultiModelCommand {
  public:
    /// Sets the new names of the resulting ConvertedModels.
    void SetResultNames(const StrVec &names) { result_names_ = names; }

    /// Returns the new names of the resulting ConvertedModels.
    const StrVec & GetResultNames() const { return result_names_; }

  protected:
    virtual void AddFields() override;
    virtual bool IsValid(Str &details) override;

    /// Useful convenience for building description strings.
    Str BuildDescription(const Str &op) const;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::VField<Str> result_names_;
    ///@}

    friend class Parser::Registry;
};
