#pragma once

#include <memory>

#include "Math/Types.h"
#include "Panes/Pane.h"
#include "SG/Typedefs.h"
#include "Util/Notifier.h"
#include "Widgets/Slider1DWidget.h"

namespace Parser { class Registry; }

/// SliderPane is a derived Pane that implements an interactive 1D slider.
class SliderPane : public Pane {
  public:
    /// Returns a Notifier that is invoked when the value of the slider
    /// changes, taking precision into account. It is passed the new value.
    Util::Notifier<float> & GetValueChanged() { return value_changed_; }

    /// Returns the current slider value.
    float GetValue() const { return cur_value_; }

    /// Redefines this to also keep the thumb the correct size.
    virtual void SetSize(const Vector2f &size) override;

    virtual bool IsInteractive()        const override { return true; }
    virtual bool IsInteractionEnabled() const override { return true; }

  protected:
    SliderPane() {}

    virtual void AddFields() override;
    virtual bool IsValid(std::string &details) override;
    virtual void CreationDone(bool is_template) override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<Vector2f> range_{"range", {0, 1}};
    Parser::TField<float>    precision_{"precision", 0};
    ///@}

    Slider1DWidgetPtr slider_;
    SG::NodePtr       thumb_;
    float             cur_value_ = 0;

    /// Notifies when the slider value changes.
    Util::Notifier<float> value_changed_;

    /// Slider callback that applies range and precision, adjusting the slider
    /// if necessary.
    void SliderChanged_();

    friend class Parser::Registry;
};

typedef std::shared_ptr<SliderPane> SliderPanePtr;
