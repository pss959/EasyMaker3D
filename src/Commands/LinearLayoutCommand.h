#pragma once

#include "Commands/MultiModelCommand.h"
#include "Memory.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(LinearLayoutCommand);

/// LinearLayoutCommand is used to lay out the currently selected Models
/// linearly with equal spacing.
///
/// \ingroup Commands
class LinearLayoutCommand : public MultiModelCommand {
  public:
    virtual std::string GetDescription() const override;

    /// Sets the offset vector.
    void SetOffset(const Vector3f &offset) { offset_ = offset; }

    /// Returns the offset vector.
    const Vector3f & GetOffset() const { return offset_; }

  protected:
    LinearLayoutCommand() {}

    virtual void AddFields() override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<Vector3f> offset_{"offset", {0, 0, 0}};
    ///@}

    friend class Parser::Registry;
};
