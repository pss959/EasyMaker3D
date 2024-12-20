//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include "Enums/PaneOrientation.h"
#include "Math/Types.h"
#include "Panes/IPaneInteractor.h"
#include "Panes/LeafPane.h"
#include "Util/Memory.h"
#include "Util/Notifier.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(SliderPane);
DECL_SHARED_PTR(Slider1DWidget);

/// SliderPane is a derived LeafPane that implements an interactive 1D slider.
///
/// \ingroup Panes
class SliderPane : public LeafPane, public IPaneInteractor {
  public:
    /// Returns a Notifier that is invoked when the slider is activated or
    /// deactivated. It is passed a flag that is true for activation.
    Util::Notifier<bool> &  GetActivation() { return activation_; }

    /// Returns a Notifier that is invoked when the value of the slider
    /// changes, taking precision into account. It is passed the new value.
    Util::Notifier<float> & GetValueChanged() { return value_changed_; }

    /// Returns the orientation. The default is PaneOrientation::kHorizontal.
    PaneOrientation GetOrientation() const { return orientation_; }

    /// Sets the range (min/max) for the value produced by the slider.
    void SetRange(const Vector2f &range) { range_ = range; }

    /// Returns the range (min/max) for the value produced by the slider. The
    /// default is (0, 1).
    const Vector2f & GetRange() const { return range_; }

    /// Sets the precision for the slider.
    void SetPrecision(float precision) { precision_ = precision; }

    /// Sets the normalized range (min/max) for the slider itself.
    void SetNormalizedSliderRange(const Vector2f &range);

    /// Returns the current slider value.
    float GetValue() const { return cur_value_; }

    /// Sets the current slider value, clamped to the current range and with
    /// precision applied. This does not notify observers.
    void SetValue(float new_value);

    // IPaneInteractor interface.
    virtual IPaneInteractor * GetInteractor() override { return this; }
    virtual ClickableWidgetPtr GetActivationWidget() const override;
    virtual BorderPtr GetFocusBorder() const override;

  protected:
    SliderPane() {}

    virtual void AddFields() override;
    virtual bool IsValid(Str &details) override;
    virtual void CreationDone() override;

    /// Redefines this to also keep the thumb the same relative size.
    virtual void UpdateForLayoutSize(const Vector2f &size) override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::EnumField<PaneOrientation> orientation_;
    Parser::TField<Vector2f>           range_;
    Parser::TField<float>              precision_;
    ///@}

    Slider1DWidgetPtr slider_;
    SG::NodePtr       thumb_;
    float             cur_value_ = 0;

    /// Notifies when the slider is activated or deactivated.
    Util::Notifier<bool>  activation_;

    /// Notifies when the slider value changes.
    Util::Notifier<float> value_changed_;

    /// Slider activation/deactivation callback.
    void SliderActivated_(bool is_activation);

    /// Slider callback that applies range and precision, adjusting the slider
    /// if necessary.
    void SliderChanged_();

    /// Adjusts the given value based on precision and clamps to the range.
    float AdjustValue_(float value) const;

    /// Updates the slider value to the given one without notifying.
    void UpdateSliderValue_(float value);

    friend class Parser::Registry;
};
