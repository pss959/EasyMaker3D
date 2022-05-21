#pragma once

#include "Base/Memory.h"
#include "Commands/MultiModelCommand.h"
#include "Models/TorusModel.h"

DECL_SHARED_PTR(ChangeTorusCommand);

/// ChangeTorusCommand is used to change the inner or outer radius of one or
/// more TorusModel instances.
///
/// \ingroup Commands
class ChangeTorusCommand : public MultiModelCommand {
  public:
    virtual std::string GetDescription() const override;

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

    /// Minimum radius value for inner radius.
    static constexpr float kMinInnerRadius = .01f;

  protected:
    ChangeTorusCommand() {}

    virtual void AddFields() override;
    virtual bool IsValid(std::string &details) override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<bool>  is_inner_radius_{"is_inner_radius", true};
    Parser::TField<float> new_radius_{"new_radius"};
    ///@}

    friend class Parser::Registry;
};
