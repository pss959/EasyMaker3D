#pragma once

#include "Base/Memory.h"
#include "Commands/MultiModelCommand.h"
#include "Enums/CSGOperation.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(ChangeCSGOperationCommand);

/// ChangeCSGOperationCommand is used to change the operation in one or more
/// CSGModel instances.
///
/// \ingroup Commands
class ChangeCSGOperationCommand : public MultiModelCommand {
  public:
    virtual Str GetDescription() const override;

    /// Sets the CSG operation to change to.
    void SetNewOperation(CSGOperation operation) { new_operation_ = operation; }

    /// Returns the new CSG operation.
    CSGOperation GetNewOperation() const { return new_operation_; }

    /// Sets the new names of the resulting Models.
    void SetResultNames(const StrVec &names) { result_names_ = names; }

    /// Returns the new names of the resulting Models.
    const StrVec & GetResultNames() const { return result_names_; }

  protected:
    ChangeCSGOperationCommand() {}

    virtual void AddFields() override;
    virtual bool IsValid(Str &details) override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::EnumField<CSGOperation> new_operation_;
    Parser::VField<Str>             result_names_;
    ///@}

    friend class Parser::Registry;
};
