#pragma once

#include "Base/Memory.h"
#include "Math/Types.h"
#include "Parser/Field.h"
#include "Util/Notifier.h"
#include "Widgets/Widget.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(ScaleWidget);
DECL_SHARED_PTR(Slider1DWidget);

/// ScaleWidget is a derived Widget that has a Slider1DWidget handle at either
/// end and a noninteractive stick joining them. Dragging either handle causes
/// the range spanned by the widget to change, resulting in a length that can
/// be used to scale some value.
///
/// There are two modes of operation:
///   - Symmetric mode: dragging a handle causes both handles to move in
///     opposite directions, leaving the current center fixed.
///   - Asymmetric mode: dragging a handle leaves the other handle fixed.
///
/// Any (but not all) of the three parts (MinHandle, MaxHandle, Stick) may be
/// disabled to not show them or allow them to interact. Note that both modes
/// have the same effect if only one handle is present.
///
/// \ingroup Widgets
class ScaleWidget : public Widget {
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
    Util::Notifier<Widget &, bool> & GetScaleChanged() {
        return scale_changed_;
    }

    /// Returns the current mode.
    Mode GetMode() const { return mode_; }

    /// Sets the current mode. This can be called during a drag to change the
    /// Widget's behavior.
    void SetMode(Mode mode) { mode_ = mode; }

    /// Returns a flag indicating whether the ScaleWidget switches modes
    /// automatically based on whether the drag starts in modified mode. If
    /// this is true, the mode will be symmetric in modified mode and
    /// asymmetric otherwise.
    bool IsUsingModifiedMode() const { return use_modified_mode_; }

    /// Returns the csale limits of the slider. Handles cannot be dragged so
    /// that the resulting scale length is outside this range.
    const Vector2f & GetLimits() const { return limits_; }

    /// Sets the scale limits of the slider. Handles cannot be dragged so that
    /// the resulting scale is outside this range.
    void SetLimits(const Vector2f &limits);

    /// Returns the value of the slider at the minimum end.
    float GetMinValue() const { return min_value_; }

    /// Returns the value of the slider at the maximum end.
    float GetMaxValue() const { return max_value_; }

    /// Sets the value of the slider at the minimum end. The value is
    /// constrained so that the resulting scale is within the scale limits.
    void SetMinValue(float value);

    /// Sets the value of the slider at the maximum end. The value is
    /// constrained so that the resulting scale is within the scale limits.
    void SetMaxValue(float value);

    /// Returns the current length between the two slider values.
    float GetLength() const { return GetMaxValue() - GetMinValue(); }

    /// Returns the Slider1DWidget on the minimum end.
    const Slider1DWidgetPtr & GetMinSlider() const { return min_slider_; }

    /// Returns the Slider1DWidget on the maximum end.
    const Slider1DWidgetPtr & GetMaxSlider() const { return max_slider_; }

  protected:
    ScaleWidget() {}

    virtual void AddFields() override;
    virtual void CreationDone() override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::EnumField<Mode>  mode_;
    Parser::TField<bool>     use_modified_mode_;
    Parser::TField<Vector2f> limits_;
    ///@}

    Util::Notifier<Widget &, bool> scale_changed_;

    Slider1DWidgetPtr min_slider_;
    Slider1DWidgetPtr max_slider_;
    SG::NodePtr       stick_;

    float min_value_ = 0;  ///< Stores the current minimum value.
    float max_value_ = 0;  ///< Stores the current maximum value.

    /// Set to true while dragging a slider handle.
    bool is_dragging_ = false;

    /// Saved center value at start of a symmetric drag.
    float starting_center_value_ = 0;

    /// Initializes and returns a slider.
    Slider1DWidgetPtr InitSlider_(const std::string &name);

    void SliderActivated_(const Slider1DWidgetPtr &slider, bool is_activation);
    void SliderChanged_(const Slider1DWidgetPtr &slider);

    /// Sets the starting configurations of the sliders when a drag is started
    // on the given slider.
    void InitForDrag_(const Slider1DWidgetPtr &slider);

    /// Updates the ranges and current values in the sliders; also updates the
    /// stick length.
    void UpdateSlidersAndStick_();

    /// Updates the stick geometry to span the current range.
    void UpdateStick_();

    friend class Parser::Registry;
};
