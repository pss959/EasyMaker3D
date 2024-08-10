#pragma once

#include "Commands/ChangeComplexityCommand.h"
#include "Tools/FloatingTool.h"
#include "Widgets/Slider1DWidget.h"

/// ComplexityTool allows the user to change the complexity of selected Models.
///
/// \ingroup Tools
class ComplexityTool : public FloatingTool {
  public:
    // ------------------------------------------------------------------------
    // Grippable interface.
    // ------------------------------------------------------------------------
    virtual void UpdateGripInfo(GripInfo &info) override;

  protected:
    ComplexityTool();

    virtual void CreationDone() override;

    virtual bool CanAttach(const Selection &sel) const override;
    virtual void Attach() override;
    virtual void Detach() override;

  private:
    /// Slider1DWidget used to modify the complexity.
    Slider1DWidgetPtr slider_;

    /// Command used to modify all affected Models.
    ChangeComplexityCommandPtr command_;

    void FindParts_();
    void SliderActivated_(bool is_activation);
    void SliderChanged_(float value);

    friend class Parser::Registry;
};
