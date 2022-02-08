#pragma once

#include <memory>

#include "Commands/MultiModelCommand.h"
#include "Models/Model.h"
#include "Util/Assert.h"

/// TranslateCommand is used to translate the currently selected Models.
///
/// \ingroup Commands
class TranslateCommand : public MultiModelCommand {
  public:
    virtual std::string GetDescription() const override;

    /// Returns the translation vector.
    const Vector3f & GetTranslation() const { return translation_; }

    /// Sets the translation vector.
    void SetTranslation(const Vector3f &trans) { translation_ = trans; }

  protected:
    TranslateCommand() {}

    virtual void AddFields() override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<Vector3f> translation_{"translation"};
    ///@}

    friend class Parser::Registry;
};

typedef std::shared_ptr<TranslateCommand> TranslateCommandPtr;
