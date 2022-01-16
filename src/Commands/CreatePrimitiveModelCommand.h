#pragma once

#include <memory>

#include "Commands/Command.h"
#include "Enums/PrimitiveType.h"
#include "Util/Assert.h"

/// CreatePrimitiveModelCommand is used to create a primitive Model of a
/// specific type.
///
/// \ingroup Commands
class CreatePrimitiveModelCommand : public Command {
  public:
    virtual std::string GetDescription() const override;

    /// Sets the type of Model.
    void SetType(PrimitiveType type) { type_ = type; }

    /// Returns the type of Model.
    PrimitiveType GetType() const { return type_; }

    /// Returns the name of the Model. This will be empty unless this Command
    /// was read from a file and a name was specified.
    const std::string & GetModelName() const { return model_name_; }

    /// Sets the name of the resulting Model.
    void SetModelName(const std::string &name) {
        ASSERT(GetModelName().empty());
        model_name_ = name;
    }

  protected:
    CreatePrimitiveModelCommand() {}

    virtual void AddFields() override;
    virtual bool IsValid(std::string &details) override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::EnumField<PrimitiveType> type_{"type", PrimitiveType::kBox};
    Parser::TField<std::string>      model_name_{"model_name"};
    ///@}

    friend class Parser::Registry;
};

typedef std::shared_ptr<CreatePrimitiveModelCommand>
    CreatePrimitiveModelCommandPtr;
