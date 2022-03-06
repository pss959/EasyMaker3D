#pragma once

#include <memory>

#include "Commands/MultiModelCommand.h"
#include "Enums/CSGOperation.h"
#include "Util/Assert.h"

/// ChangeCSGOperationCommand is used to change the operation in one or more
/// CSGModel instances.
/// \ingroup Commands
class ChangeCSGOperationCommand : public MultiModelCommand {
  public:
    virtual std::string GetDescription() const override;

    /// Sets the CSG operation to change to.
    void SetNewOperation(CSGOperation operation) { new_operation_ = operation; }

    /// Returns the new CSG operation.
    CSGOperation GetNewOperation() const { return new_operation_; }

    /// Sets the new names of the resulting Models.
    void SetResultNames(const std::vector<std::string> &names);

    /// Returns the new names of the resulting Models.
    const std::vector<std::string> & GetResultNames() const {
        return result_names_;
    }

  protected:
    ChangeCSGOperationCommand() {}

    virtual void AddFields() override;
    virtual bool IsValid(std::string &details) override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::EnumField<CSGOperation> new_operation_{"new_operation",
                                                   CSGOperation::kUnion};
    Parser::VField<std::string>     result_names_{"result_names"};
    ///@}

    friend class Parser::Registry;
};

typedef std::shared_ptr<ChangeCSGOperationCommand> ChangeCSGOperationCommandPtr;
