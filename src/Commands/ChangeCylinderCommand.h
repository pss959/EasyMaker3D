#pragma once

#include "Commands/MultiModelCommand.h"
#include "Memory.h"
#include "Models/CylinderModel.h"

DECL_SHARED_PTR(ChangeCylinderCommand);

/// ChangeCylinderCommand is used to change the top or bottom radius of one or
/// more CylinderModel instances.
/// \ingroup Commands
class ChangeCylinderCommand : public MultiModelCommand {
  public:
    virtual std::string GetDescription() const override;

    /// Sets which radius is being changed.
    void SetWhichRadius(CylinderModel::Radius which) { which_radius_ = which; }

    /// Returns which radius is being changed.
    CylinderModel::Radius GetWhichRadius() const { return which_radius_; }

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
    Parser::EnumField<CylinderModel::Radius> which_radius_{
        "which_radius", CylinderModel::Radius::kTop};
    Parser::TField<float> new_radius_{"new_radius"};
    ///@}

    friend class Parser::Registry;
};
