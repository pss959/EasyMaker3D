#pragma once

#include "Commands/CombineCommand.h"
#include "Enums/CSGOperation.h"
#include "Util/Memory.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(CombineCSGModelCommand);

/// CombineCSGModelCommand is used to create a CSG Model of a
/// specific type.
///
/// \ingroup Commands
class CombineCSGModelCommand : public CombineCommand {
  public:
    virtual Str GetDescription() const override;

    /// Sets the CSG operation.
    void SetOperation(CSGOperation operation) { operation_ = operation; }

    /// Returns the CSG operation.
    CSGOperation GetOperation() const { return operation_; }

  protected:
    CombineCSGModelCommand() {}

    virtual void AddFields() override;
    virtual bool IsValid(Str &details) override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::EnumField<CSGOperation> operation_;
    ///@}

    friend class Parser::Registry;
};
