#pragma once

#include <vector>

#include "Commands/MultiModelCommand.h"
#include "Util/Memory.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(PasteCommand);

/// PasteCommand pastes Models from the clipboard into the scene. The target
/// can be a ParentModel (for paste-into) or nothing, in which case the Models
/// are pasted as top-level Models.
///
/// Paste initially operates on what is in the clipboard, but then retains that
/// information for Redo so that the same Models are pasted the next time,
/// regardless of what is in the clipboard.
///
/// \ingroup Commands
class PasteCommand : public Command {
  public:
    virtual Str GetDescription() const override;

    /// Sets the name of the ParentModel to paste the Models into. The name is
    /// empty by default, meaning that Models are pasted as top-level Models.
    void SetParentName(const Str &name) { parent_name_ = name; }

    /// Returns the name of the ParentModel to paste the Models into. The name
    /// is empty by default, meaning that Models are pasted as top-level
    /// Models.
    Str GetParentName() const { return parent_name_; }

    /// This can be used to set the Models that were pasted for use by
    /// GetDescription(). Otherwise, there would be no way to show what was
    /// pasted.
    void SetModelsForDescription(const std::vector<ModelPtr> &models);

  protected:
    PasteCommand() {}

    virtual void AddFields() override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<Str> parent_name_;
    ///@}

    /// Stores the names of Models for GetDescription().
    StrVec model_names_;

    friend class Parser::Registry;
};
