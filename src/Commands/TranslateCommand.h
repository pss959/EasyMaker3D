#pragma once

#include "Commands/MultiModelCommand.h"
#include "Util/Memory.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(TranslateCommand);

/// TranslateCommand is used to translate the currently selected Models.
///
/// \ingroup Commands
class TranslateCommand : public MultiModelCommand {
  public:
    virtual Str GetDescription() const override;

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
    Parser::TField<Vector3f> translation_;
    ///@}

    friend class Parser::Registry;
};
