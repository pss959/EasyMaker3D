#pragma once

#include "Math/Linear.h"
#include "Math/Types.h"
#include "Parser/Field.h"
#include "Util/Notifier.h"
#include "Widgets/DraggableWidget.h"

/// SliderWidgetBase is an abstract base class for the Slider1DWidget and
/// Slider2DWidget classes that consolidates shared code. The template
/// parameter is the type of value stored and returned by the slider.
/// \ingroup Widgets
template <typename T> class SliderWidgetBase : public DraggableWidget {
  public:
    virtual void AddFields() override;
    virtual void CreationDone() override;

    /// Returns a Notifier that is invoked when the user drags the widget
    /// causing the value to change. It is passed the widget and the new value.
    Util::Notifier<Widget&, const T &> & GetValueChanged() {
        return value_changed_;
    }

    /// Returns a flag indicating whether the slider will react to changes in
    /// the current Precision setting, moving less when the Precision is
    /// finer. The default is false.  (Note that it does not affect the actual
    /// value of the slider.)
    bool IsPrecisionBased() const { return is_precision_based_; }

    /// Returns a flag indicating whether the value returned by GetValue() and
    /// the value passed to the GetValueChanged() observers will be normalized
    /// to the range [0,1] instead of ranging between the minimum to maximum
    /// values. The default is false.
    bool IsNormalized() const { return is_normalized_; }

    /// Returns the minimum value for the slider. The default value is 0.
    const T & GetMinValue() const { return min_value_; }

    /// Returns the maximum value for the slider. The default value is 1.
    const T & GetMaxValue() const { return max_value_; }

    /// Returns the value used to initialize the widget.
    const T & GetInitialValue() const { return initial_value_; }

    /// Returns the current value of the slider, normalized if IsNormalized()
    /// is true.
    const T & GetValue() const { return value_; }

    /// Returns the unnormalized value of the slider.
    T GetUnnormalizedValue() const;

    /// Sets the range for the slider.
    void SetRange(const T &min_value, const T &max_value);

    /// Sets the current value of the slider. If IsNormalized() is false, the
    /// value is clamped to the minimum and maximum values; if it is true, it
    /// is clamped to [0,1].
    void SetValue(const T &value);

    virtual void StartDrag(const DragInfo &info) override;
    virtual void ContinueDrag(const DragInfo &info) override;
    virtual void EndDrag() override;

  protected:
    /// Derived classes can use this constant to scale grip drags to make arm
    /// motion reasonable.
    static constexpr float kGripDragScale = 80.f;

    /// Derived classes must implement this to compute a value from
    /// GetMinValue(), GetMaxValue() and GetValue(). This is used only when
    /// IsNormalized() is true.
    virtual T GetInterpolated() const = 0;

    /// Derived classes must implement this to compute an absolute value for a
    /// pointer drag using the given ray (in local coordinates).
    virtual T GetRayValue(const Ray &local_ray) = 0;

    /// Derived classes must implement this to compute a value for a grip drag
    /// using the the start and current controller positions (in world
    /// coordinates).
    virtual T GetGripValue(const T &start_value,
                           const Point3f &p0, const Point3f &p1) = 0;

    /// Updates the translation of the widget based on the current value.
    virtual void UpdatePosition() = 0;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<bool> is_precision_based_{"is_precision_based", false};
    Parser::TField<bool> is_normalized_{"is_normalized", false};
    Parser::TField<T>    min_value_{"min_value", ZeroInit<T>()};
    Parser::TField<T>    max_value_{"max_value", OneInit<T>()};
    Parser::TField<T>    initial_value_{"initial_value", ZeroInit<T>()};
    ///@}

    /// Stores the current unnormalized value.
    T value_ = ZeroInit<T>();

    /// Value of the slider when a drag starts or when precision changes during
    /// a precision-based drag.
    T start_value_;

    /// For a pointer drag, this is the computed absolute value at the start of
    /// the drag.
    T start_ray_value_;

    /// Current precision during a drag. Initialized to 0 so that any real
    /// precision will be considered a change.
    float precision_ = 0;

    /// Notifies when the widget value changes.
    Util::Notifier<Widget&, const T &> value_changed_;

    /// Computes the value resulting from a drag.
    T ComputeDragValue_(const DragInfo &info);
};
