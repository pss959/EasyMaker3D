#pragma once

#include "Commands/MultiModelCommand.h"
#include "Util/Memory.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(ChangeTorusCommand);

/// ChangeTorusCommand is used to change the inner or outer radius of one or
/// more TorusModel instances.
///
/// \ingroup Commands
class ChangeTorusCommand : public MultiModelCommand {
  public:
    virtual Str GetDescription() const override;

    /// Sets a flag indicating which radius is being changed. The default is
    /// true.
    void SetIsInnerRadius(bool is_inner) { is_inner_radius_ = is_inner; }

    /// Returns a flag indicating which radius is being changed. The default is
    /// true.
    bool IsInnerRadius() const { return is_inner_radius_; }

    /// Sets the new radius in stage coordinates.
    void SetNewRadius(float radius) { new_radius_ = radius; }

    /// Returns the new radius in stage coordinates.
    float GetNewRadius() const { return new_radius_; }

  protected:
    ChangeTorusCommand() {}

    virtual void AddFields() override;
    virtual bool IsValid(Str &details) override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<bool>  is_inner_radius_;
    Parser::TField<float> new_radius_;
    ///@}

    friend class Parser::Registry;
};
