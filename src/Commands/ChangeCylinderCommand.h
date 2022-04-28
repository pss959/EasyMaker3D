#pragma once

#include "Commands/MultiModelCommand.h"
#include "Memory.h"
#include "Models/CylinderModel.h"

DECL_SHARED_PTR(ChangeCylinderCommand);

/// ChangeCylinderCommand is used to change the top or bottom radius of one or
/// more CylinderModel instances.
///
/// \ingroup Commands
class ChangeCylinderCommand : public MultiModelCommand {
  public:
    virtual std::string GetDescription() const override;

    /// Sets a flag indicating which radius is being changed. The default is
    /// true.
    void SetIsTopRadius(bool is_top) { is_top_radius_ = is_top; }

    /// Returns a flag indicating which radius is being changed. The default is
    /// true.
    bool IsTopRadius() const { return is_top_radius_; }

    /// Sets the new radius in stage coordinates.
    void SetNewRadius(float radius) { new_radius_ = radius; }

    /// Returns the new radius in stage coordinates.
    float GetNewRadius() const { return new_radius_; }

  protected:
    ChangeCylinderCommand() {}

    virtual void AddFields() override;
    virtual bool IsValid(std::string &details) override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<bool>  is_top_radius_{"is_top_radius", true};
    Parser::TField<float> new_radius_{"new_radius"};
    ///@}

    friend class Parser::Registry;
};
