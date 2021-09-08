#pragma once

#include "Math/Types.h"
#include "Parser/Field.h"
#include "Widgets/IDraggableWidget.h"
#include "Widgets/Widget.h"

//! SliderWidgetBase is an abstract base class for the Slider1DWidget and
//! Slider2DWidget classes that consolidates shared code. The template
//! parameter is the type of value stored and returned by the slider.
//! \ingroup Widgets
template <typename T> class SliderWidgetBase : public Widget,
                                               public IDraggableWidget {
  public:
    virtual void AddFields() override {
        Widget::AddFields();
        AddField(min_value_);
        AddField(max_value_);
    }

    //! Returns a Notifier that is invoked when the user drags the widget
    //! causing the value to change. It is passed the widget and the new value.
    Util::Notifier<Widget&, const T &> & GetValueChanged() {
        return value_changed_;
    }

    //! Returns the minimum value for the slider. The default value is 0.
    const T & GetMinValue() { return min_value_; }

    //! Returns the maximum value for the slider. The default value is 1.
    const T & GetMaxValue() { return max_value_; }

    //! When set to true, GetCurrentValue() and the value passed to the
    // GetValuechanged() observers will be normalized to the range (0,1)
    // instead of from the minimum to maximum values. The default is false.
    void SetNormalized(bool normalized) { is_normalized_ = normalized; }

    //! Returns the last SetNormalized() setting.
    bool IsNormalized() const { return is_normalized_; }

    //! Returns the current value of the slider, normalized if
    // requested.
    const T & GetCurrentValue() { return current_value_; }

    //! Sets the current value of the slider. This is clamped to the minimum
    //! and maximum values if not normalized and (0,1) if normalized.
    void SetCurrentValue(const T &value) {
        if (IsNormalized())
            current_value_ = Clamp(value, T(0), T(1));
        else
            current_value_ = Clamp(value, min_value_, max_value_);
        UpdatePosition();
        value_changed_.Notify(this, current_value_);
    }

  protected:
    //! Derived classes can use this constant to scale grip drags to make arm
    // motion reasonable.
    static constexpr float kGripDragScale = 80.f;

    //! Adjusts the given value based on the current minValue, maxValue, and
    // isNormalized flag.
    virtual T AdjustValue(T value);

    //! Derived classes must implement this to compute a value when
    // IsNormalized() is true.
    virtual T Interpolate() = 0;

    //! Tells the derived class that a drag is beginning.
    virtual void PrepareForDrag(const DragInfo &info,
                                const Vector3f &start_point) = 0;

    //! Computes the value resulting from a drag. The starting 3D point is
    // passed in (needed if a grip drag) along with the slider value at the
    // start of the drag. If the precision value is positive, it should be used
    // to affect the motion of the slider.
    virtual T ComputeDragValue(const DragInfo &info,
                               const Vector3f &start_point,
                               const T start_value, float precision) = 0;

    //! Updates the translation of the widget based on the current value.
    virtual void UpdatePosition() = 0;

  private:
    // ------------------------------------------------------------------------
    // Variables.

    //! \name Parsed Fields
    //!@{
    Parser::TField<T> min_value_{"min_value", T(0)};
    Parser::TField<T> max_value_{"max_value", T(1)};
    //!@}

    T current_value_ = T(0);

    bool is_normalized_ = false;

    //! Notifies when the widget value changes.
    Util::Notifier<Widget&, const T &> value_changed_;

    // ------------------------------------------------------------------------
    // Functions.
};
