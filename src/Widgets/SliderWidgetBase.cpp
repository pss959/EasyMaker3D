#include "Widgets/SliderWidgetBase.h"

template <typename T>
void SliderWidgetBase<T>::AddFields() {
    AddField(is_precision_based_.Init("is_precision_based", false));
    AddField(is_normalized_.Init("is_normalized",           false));
    AddField(min_value_.Init("min_value",                   ZeroInit<T>()));
    AddField(max_value_.Init("max_value",                   OneInit<T>()));
    AddField(initial_value_.Init("initial_value",           ZeroInit<T>()));

    Widget::AddFields();
}

template <typename T>
void SliderWidgetBase<T>::CreationDone() {
    Widget::CreationDone();

    if (! IsTemplate())
        SetValue(initial_value_);
}

template <typename T>
T SliderWidgetBase<T>::GetUnnormalizedValue() const {
    return IsNormalized() ? GetInterpolated() : value_;
}

template <typename T>
void SliderWidgetBase<T>::SetRange(const T &min_value, const T &max_value) {
    min_value_ = min_value;
    max_value_ = max_value;
}

template <typename T>
void SliderWidgetBase<T>::SetValue(const T &value) {
    value_ = IsNormalized() ? Clamp(value, ZeroInit<T>(), OneInit<T>()) :
        Clamp(value, min_value_, max_value_);
    UpdatePosition();
    value_changed_.Notify(*this, value_);
}

template <typename T>
void SliderWidgetBase<T>::StartDrag(const DragInfo &info) {
    DraggableWidget::StartDrag(info);

    // Pointer drags use absolute positions to compute slider values. To turn
    // that into a relative change from the current position, save the
    // (absolute) value for the starting position. This will be subtracted to
    // get a relative value change in ComputeDragValue_().
    if (info.trigger == Trigger::kPointer)
        start_ray_value_ = GetLocalRayValue_(info);

    start_value_ = GetUnnormalizedValue();
    precision_   = 0;  // Invalid value so changes will be processed.
    SetActive(true);
}

template <typename T>
void SliderWidgetBase<T>::ContinueDrag(const DragInfo &info) {
    DraggableWidget::ContinueDrag(info);

    // If reacting to precision, check for a change in current precision
    // and reset the starting drag values if it changed.
    if (IsPrecisionBased() && info.linear_precision != precision_) {
        precision_   = info.linear_precision;
        start_value_ = GetUnnormalizedValue();
    }

    // Compute the new value.
    value_ = ComputeDragValue_(info);
    UpdatePosition();
    value_changed_.Notify(*this, value_);
}

template <typename T>
void SliderWidgetBase<T>::EndDrag() {
    SetActive(false);
}

template <typename T>
T SliderWidgetBase<T>::ComputeDragValue_(const DragInfo &info) {
    T val;
    if (info.trigger == Trigger::kPointer) {
        // For a pointer drag, compute the new value as the closest position to
        // the new pointer ray.
        val = start_value_ + GetLocalRayValue_(info) - start_ray_value_;
    }
    else if (info.trigger == Trigger::kGrip) {
        // For a grip drag, use the change in world coordinates along the
        // slider direction to get the base change in value.
        val = start_value_ + GetGripDragScale() *
            GetControllerMotion(GetStartDragInfo().grip_position,
                                info.grip_position);
    }
    else {
        // For a touch drag, use the change in world coordinates along the
        // slider direction to get the base change in value.
        val = start_value_ +
            GetControllerMotion(GetStartDragInfo().touch_position,
                                info.touch_position);
    }

    // If this is precision-based, use the precision value to scale the
    // change in value.
    if (IsPrecisionBased() && info.linear_precision > 0)
        val = start_value_ + info.linear_precision * (val - start_value_);
    val = Clamp(val, GetMinValue(), GetMaxValue());

    if (IsNormalized())
        val = (val - GetMinValue()) / (GetMaxValue() - GetMinValue());

    return val;
}

template <typename T>
T SliderWidgetBase<T>::GetLocalRayValue_(const DragInfo &info) {
    return GetRayValue(Ray(WorldToWidget(info.ray.origin),
                           WorldToWidget(info.ray.direction)));
}

// ----------------------------------------------------------------------------
// Instantiate for supported types.
// ----------------------------------------------------------------------------

template class SliderWidgetBase<float>;
template class SliderWidgetBase<Vector2f>;
