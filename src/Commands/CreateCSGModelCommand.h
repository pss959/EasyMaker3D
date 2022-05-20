﻿#pragma once

#include "Base/Memory.h"
#include "Commands/CombineCommand.h"
#include "Enums/CSGOperation.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(CreateCSGModelCommand);

/// CreateCSGModelCommand is used to create a CSG Model of a
/// specific type.
///
/// \ingroup Commands
class CreateCSGModelCommand : public CombineCommand {
  public:
    virtual std::string GetDescription() const override;

    /// Sets the CSG operation.
    void SetOperation(CSGOperation operation) { operation_ = operation; }

    /// Returns the CSG operation.
    CSGOperation GetOperation() const { return operation_; }

  protected:
    CreateCSGModelCommand() {}

    virtual void AddFields() override;
    virtual bool IsValid(std::string &details) override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::EnumField<CSGOperation> operation_{"operation",
                                               CSGOperation::kUnion};
    ///@}

    friend class Parser::Registry;
};
