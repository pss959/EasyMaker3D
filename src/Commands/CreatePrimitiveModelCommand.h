#pragma once

#include "Commands/CreateModelCommand.h"
#include "Enums/PrimitiveType.h"
#include "Util/Memory.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(CreatePrimitiveModelCommand);

/// CreatePrimitiveModelCommand is used to create a primitive Model of a
/// specific type.
///
/// \ingroup Commands
class CreatePrimitiveModelCommand : public CreateModelCommand {
  public:
    virtual Str GetDescription() const override;

    /// Sets the type of Model.
    void SetType(PrimitiveType type) { type_ = type; }

    /// Returns the type of Model.
    PrimitiveType GetType() const { return type_; }

  protected:
    CreatePrimitiveModelCommand() {}

    virtual void AddFields() override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::EnumField<PrimitiveType> type_;
    ///@}

    friend class Parser::Registry;
};
