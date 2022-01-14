#pragma once

#include <memory>

#include "Math/Types.h"
#include "Panes/Pane.h"
#include "SG/Typedefs.h"
#include "Widgets/Slider1DWidget.h"

namespace Parser { class Registry; }

/// SliderPane is a derived Pane that implements an interactive 1D slider.
class SliderPane : public Pane {
  public:
    virtual void AddFields() override;
    virtual bool IsValid(std::string &details) override;
    virtual void AllFieldsParsed(bool is_template) override;

    /// Returns the current slider value.
    float GetValue() const;

    /// Sets the current value of the slider.
    void SetValue(float value);

    /// Redefines this to also keep the thumb the correct size.
    virtual void SetSize(const Vector2f &size) override;

    virtual bool IsInteractive()        const override { return true; }
    virtual bool IsInteractionEnabled() const override { return true; }

  protected:
    SliderPane() {}

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<Vector2f> range_{"range", {0, 1}};
    Parser::TField<float>    precision_{"precision", 0};
    ///@}

    Slider1DWidgetPtr slider_;
    SG::NodePtr       thumb_;

    /// Slider callback that applies range and precision, adjusting the slider
    /// if necessary.
    void UpdateValue_();

    friend class Parser::Registry;
};

typedef std::shared_ptr<SliderPane> SliderPanePtr;
