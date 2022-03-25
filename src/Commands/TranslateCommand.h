#pragma once

#include "Commands/MultiModelCommand.h"
#include "Memory.h"

DECL_SHARED_PTR(TranslateCommand);

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
    Parser::TField<Vector3f> translation_{"translation", {0, 0, 0}};
    ///@}

    friend class Parser::Registry;
};
