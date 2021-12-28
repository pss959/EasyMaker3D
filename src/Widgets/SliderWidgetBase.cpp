#include "Widgets/SliderWidgetBase.h"

template <typename T>
void SliderWidgetBase<T>::AddFields() {
    Widget::AddFields();
    AddField(is_precision_based_);
    AddField(is_normalized_);
    AddField(min_value_);
    AddField(max_value_);
    AddField(initial_value_);
}

template <typename T>
bool SliderWidgetBase<T>::IsValid(std::string &details) {
    if (! DraggableWidget::IsValid(details))
        return false;
    SetValue(initial_value_);
    return true;
}

template <typename T>
T SliderWidgetBase<T>::GetUnnormalizedValue() const {
    return IsNormalized() ? GetInterpolated() : value_;
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

    start_value_ = GetUnnormalizedValue();
    precision_   = 0;  // Invalid value so changes will be processed.
    SetActive(true);
}

template <typename T>
void SliderWidgetBase<T>::ContinueDrag(const DragInfo &info) {
    // If reacting to precision, check for a change in current precision
    // and reset the starting drag values if it changed.
    if (IsPrecisionBased() && info.linear_precision != precision_) {
        precision_   = info.linear_precision;
        start_value_ = GetUnnormalizedValue();
    }

    // Compute the new value.
    value_ = ComputeDragValue_(info, start_value_);
    UpdatePosition();
    value_changed_.Notify(*this, value_);
}

template <typename T>
void SliderWidgetBase<T>::EndDrag() {
    SetActive(false);
}

template <typename T>
T SliderWidgetBase<T>::ComputeDragValue_(const DragInfo &info,
                                         const T &start_value) {
    // For a grip drag, use the change in world coordinates along the
    // slider direction to get the base change in value. For a pointer
    // drag, just compute the new value as the closest position to the
    // pointer ray.
    T val = info.is_grip ?
        GetGripValue(start_value,
                     GetStartDragInfo().grip_position, info.grip_position) :
        GetRayValue(Ray(ToLocal(info.ray.origin),
                        ToLocal(info.ray.direction)));

    // If this is precision-based, use the precision value to scale the
    // change in value.
    if (IsPrecisionBased() && info.linear_precision > 0)
        val = start_value + info.linear_precision * (val - start_value);
    val = Clamp(val, GetMinValue(), GetMaxValue());

    if (IsNormalized())
        val = (val - GetMinValue()) / (GetMaxValue() - GetMinValue());

    return val;
}

// ----------------------------------------------------------------------------
// Instantiate for supported types.
// ----------------------------------------------------------------------------

template class SliderWidgetBase<float>;
template class SliderWidgetBase<Vector2f>;
