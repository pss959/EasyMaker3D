#pragma once

#include <memory>

#include "Assert.h"
#include "Commands/MultiModelCommand.h"
#include "Models/Model.h"

/// TranslateCommand is used to translate the currently selected Models.
///
/// \ingroup Commands
class TranslateCommand : public MultiModelCommand {
    virtual void AddFields() override;
    virtual std::string GetDescription() const override;

    /// Returns the translation vector.
    const Vector3f & GetTranslation() const { return translation_; }

  protected:
    TranslateCommand() {}

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<Vector3f> translation_{"translation"};
    ///@}

    friend class Parser::Registry;
};

typedef std::shared_ptr<TranslateCommand> TranslateCommandPtr;
