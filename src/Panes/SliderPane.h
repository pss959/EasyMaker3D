#pragma once

#include "Math/Types.h"
#include "Memory.h"
#include "Panes/Pane.h"
#include "Util/Notifier.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(SliderPane);
DECL_SHARED_PTR(Slider1DWidget);

/// SliderPane is a derived Pane that implements an interactive 1D slider.
class SliderPane : public Pane {
  public:
    enum class Orientation {
        kHorizontal,  ///< Slider has minimum at left, maximum at right.
        kVertical,    ///< Slider has minimum at bottom, maximum at top.
    };

    /// Returns a Notifier that is invoked when the slider is activated or
    /// deactivated. It is passed a flag that is true for activation.
    Util::Notifier<bool> &  GetActivation() { return activation_; }

    /// Returns a Notifier that is invoked when the value of the slider
    /// changes, taking precision into account. It is passed the new value.
    Util::Notifier<float> & GetValueChanged() { return value_changed_; }

    /// Returns the orientation. The default is Orientation::kHorizontal.
    Orientation GetOrientation() const { return orientation_; }

    /// Sets the range (min/max) for the slider.
    void SetRange(const Vector2f &range) { range_ = range; }

    /// Sets the precision for the slider.
    void SetPrecision(float precision) { precision_ = precision; }

    /// Returns the current slider value.
    float GetValue() const { return cur_value_; }

    /// Sets the current slider value, clamped to the current range and with
    /// precision applied. This does not notify observers.
    void SetValue(float new_value);

    /// Redefines this to also keep the thumb the correct size.
    virtual void SetSize(const Vector2f &size) override;

    virtual bool IsInteractive()        const override { return true; }
    virtual bool IsInteractionEnabled() const override { return true; }

  protected:
    SliderPane() {}

    virtual void AddFields() override;
    virtual bool IsValid(std::string &details) override;
    virtual void CreationDone() override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::EnumField<Orientation> orientation_{"orientation",
                                                Orientation::kHorizontal};
    Parser::TField<Vector2f>       range_{"range", {0, 1}};
    Parser::TField<float>          precision_{"precision", 0};
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
