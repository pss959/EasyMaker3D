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
    virtual void AddFields() override {
        Widget::AddFields();
        AddField(is_normalized_);
        AddField(min_value_);
        AddField(max_value_);
        AddField(initial_value_);
    }
    virtual bool IsValid(std::string &details) override {
        if (! DraggableWidget::IsValid(details))
            return false;
        SetValue(initial_value_);
        return true;
    }

    /// Returns a Notifier that is invoked when the user drags the widget
    /// causing the value to change. It is passed the widget and the new value.
    Util::Notifier<Widget&, const T &> & GetValueChanged() {
        return value_changed_;
    }

    /// Sets a flag indicating whether the slider will react to changes in the
    /// current Precision setting, moving less when the Precision is finer. The
    /// default is false.  (Note that it does not affect the actual value of
    /// the slider.)
    void SetIsPrecisionBased(bool is_pb) { is_precision_based_ = is_pb; }

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
    T GetUnnormalizedValue() const {
        return IsNormalized() ? GetInterpolated() : value_;
    }

    /// Sets the current value of the slider. If IsNormalized() is false, the
    /// value is clamped to the minimum and maximum values; if it is true, it
    /// is clamped to [0,1].
    void SetValue(const T &value) {
        value_ = IsNormalized() ? Clamp(value, ZeroInit<T>(), OneInit<T>()) :
            Clamp(value, min_value_, max_value_);
        UpdatePosition();
        value_changed_.Notify(*this, value_);
    }

    virtual void StartDrag(const DragInfo &info) override {
        SavePathToThis(info);
        start_drag_point_ = info.hit.point;
        start_drag_value_ = GetUnnormalizedValue();
        precision_        = 0;  // Invalid value so changes will be processed.
        SetActive(true);
        PrepareForDrag(info, start_drag_point_);
    }

    virtual void ContinueDrag(const DragInfo &info) override {
        // If reacting to precision, check for a change in current precision
        // and reset the starting drag values if it changed.
        if (IsPrecisionBased() && info.linear_precision != precision_) {
            precision_        = info.linear_precision;
            start_drag_value_ = GetUnnormalizedValue();
        }

        // Temporarily reset the transform so that everything is in the correct
        // local coordinates.
        const Vector3f  saved_scale = GetScale();
        const Rotationf saved_rot   = GetRotation();
        SetScale(Vector3f(1, 1, 1));
        SetRotation(Rotationf::Identity());

        // Compute the new coordinates, apply precision if requested, and clamp
        // the result.
        value_ = ComputeDragValue(info, start_drag_point_, start_drag_value_,
                                  (IsPrecisionBased() ? precision_ : 0.f));
        UpdatePosition();
        value_changed_.Notify(*this, value_);

        // Restore the scale and rotation. The position was already set.
        SetScale(saved_scale);
        SetRotation(saved_rot);
    }

    virtual void EndDrag() override {
        SetActive(false);
    }

  protected:
    /// Derived classes can use this constant to scale grip drags to make arm
    /// motion reasonable.
    static constexpr float kGripDragScale = 80.f;

    /// Derived classes must implement this to compute a value from
    /// GetMinValue(), GetMaxValue() and GetValue(). This is used only when
    /// IsNormalized() is true.
    virtual T GetInterpolated() const = 0;

    /// Tells the derived class that a drag is beginning.
    virtual void PrepareForDrag(const DragInfo &info,
                                const Point3f &start_point) = 0;

    /// Computes the value resulting from a drag. The starting 3D point is
    /// passed in (needed if a grip drag) along with the slider value at the
    /// start of the drag. If the precision value is positive, it should be
    /// used to affect the motion of the slider.
    virtual T ComputeDragValue(const DragInfo &info,
                               const Point3f &start_point,
                               const T &start_value, float precision) = 0;

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
    T start_drag_value_;

    /// World-space location of the controller at the start of a drag, used for
    /// grip dragging.
    Point3f start_drag_point_{0, 0, 0};

    /// Current precision during a drag. Initialized to 0 so that any real
    /// precision will be considered a change.
    float precision_ = 0;

    /// Notifies when the widget value changes.
    Util::Notifier<Widget&, const T &> value_changed_;
};
