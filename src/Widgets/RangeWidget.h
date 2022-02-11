#pragma once

#include <memory>

#include "Math/Types.h"
#include "Parser/Field.h"
#include "Util/Notifier.h"
#include "Widgets/Slider1DWidget.h"
#include "Widgets/Widget.h"

namespace Parser { class Registry; }

/// RangeWidget is a derived Widget that has a Slider1DWidget handle at either
/// end and a noninteractive stick joining them. Dragging either handle causes
/// the range spanned by the widget to change.
///
/// There are two modes of operation:
///   - Symmetric mode: dragging a handle causes both handles to move in
///     opposite directions, leaving the current center fixed.
///   - Asymmetric mode: dragging a handle leaves the other handle fixed.
///
/// Any (but not all) of the three parts (MinHandle, MaxHandle, Stick) may be
/// disabled to not show them or allow them to interact. Note that both modes
/// have the same effect if only one handle is present.
/// \ingroup Widgets
class RangeWidget : public Widget {
  public:
    /// Defines the mode for the widget.
    enum class Mode {
        kSymmetric,   ///< Handles move symmetrically to scale about the center.
        kAsymmetric,  ///< Handle moves independently to scale about other end.
    };

    /// Returns a Notifier that is invoked when the user drags a handle.  It is
    /// passed the widget and a flag that is true when the slider for the
    /// maximum value resulted in the change and false when the minimum slider
    /// resulted in the change.
    Util::Notifier<Widget &, bool> & GetRangeChanged() {
        return range_changed_;
    }

    /// Returns the current mode.
    Mode GetMode() const { return mode_; }

    /// Sets the current mode. This can be called during a drag to change the
    /// Widget's behavior.
    void SetMode(Mode mode) { mode_ = mode; }

    /// Returns the length limits of the slider. Handles cannot be dragged so
    /// that the length is outside this range.
    const Vector2f & GetLengthLimits() const { return length_limits_; }

    /// Sets the length limits of the slider. Handles cannot be dragged so
    /// that the length is outside this range.
    void SetLengthLimits(const Vector2f &limits);

    /// Returns the current range consisting of the values of the minimum and
    /// maximum sliders.
    Vector2f GetRange() const {
        return Vector2f(min_slider_->GetValue(), max_slider_->GetValue());
    }

    /// Sets the current range consisting of the values of the minimum and
    /// maximum sliders. The maximum value is constrained so that the range
    /// stays within the length limits.
    void SetRange(const Vector2f &range);

    /// Returns the Slider1DWidget on the minimum end.
    Slider1DWidget & GetMinSlider() const { return *min_slider_; }

    /// Returns the Slider1DWidget on the maximum end.
    Slider1DWidget & GetMaxSlider() const { return *max_slider_; }

  protected:
    RangeWidget() {}

    virtual void AddFields() override;
    virtual void CreationDone() override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::EnumField<Mode>  mode_{"mode", { Mode::kAsymmetric }};
    Parser::TField<Vector2f> length_limits_{"length_limits", { .01f, 100.f }};
    ///@}

    Util::Notifier<Widget &, bool> range_changed_;

    Slider1DWidgetPtr min_slider_;
    Slider1DWidgetPtr max_slider_;
    SG::NodePtr       stick_;

    /// Saved center value at start of a symmetric drag.
    float starting_center_value_ = 0;

    /// Makes sure the slider values are within the length limits.
    void ClampLength_();

    /// Initializes and returns a slider.
    Slider1DWidgetPtr InitSlider_(const std::string &name);

    /// Updates the ranges in the sliders and stick to match the current range.
    void UpdateSliderRanges_();

    void SliderActivated_(const Slider1DWidgetPtr &slider, bool is_activation);
    void SliderChanged_(const Slider1DWidgetPtr &slider);

    /// Sets the starting configurations of the sliders when a drag is started
    // on the given slider.
    void InitForDrag_(const Slider1DWidgetPtr &slider);

    /// Updates the stick geometry to span the current range.
    void UpdateStick_();

    friend class Parser::Registry;
};

typedef std::shared_ptr<RangeWidget> RangeWidgetPtr;
