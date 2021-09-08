#pragma once

#include "Widgets/SliderWidgetBase.h"

//! Slider1DWidget is a derived SliderWidgetBase that provides interaction
//! along a constrained linear path.
//!
//! The direction of motion is defined by default to be along the X axis.
class Slider1DWidget : public SliderWidgetBase<float> {
  public:
    virtual void StartDrag(const DragInfo &info) override;
    virtual void ContinueDrag(const DragInfo &info) override;
    virtual void EndDrag() override;

  private:
    // ------------------------------------------------------------------------
    // Variables.

    //! \name Parsed Fields
    //!@{
    // XXXX Anything?
    //!@}

    // ------------------------------------------------------------------------
    // Functions.
};
