#pragma once

#include <memory>

#include "Commands/Command.h"
#include "Enums/PrimitiveType.h"

/// CreatePrimitiveModelCommand is used to create a primitive Model of a
/// specific type.
///
/// \ingroup Commands
class CreatePrimitiveModelCommand : public Command {
  public:
    /// Default constructor for reading.
    CreatePrimitiveModelCommand() {}

    /// Constructor that is passed the type of primitive to create.
    CreatePrimitiveModelCommand(PrimitiveType type) { type_ = type; }

    virtual void AddFields() override;
    virtual bool IsValid(std::string &details) override;
    virtual std::string GetDescription() const override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::EnumField<PrimitiveType> type_{"type"};
    Parser::TField<std::string>      model_name_{"model_name"};
    ///@}
};

typedef std::shared_ptr<CreatePrimitiveModelCommand>
    CreatePrimitiveModelCommandPtr;
