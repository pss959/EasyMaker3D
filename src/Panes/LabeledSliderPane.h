#pragma once

#include <memory>

#include "Math/Types.h"
#include "Panes/BoxPane.h"
#include "Panes/SliderPane.h"
#include "Panes/TextPane.h"

namespace Parser { class Registry; }

/// LabeledSliderPane is a derived BoxPane that adds text labels to a
/// SliderPane to show the minimum, maximum, and current values.
class LabeledSliderPane : public BoxPane {
  public:
    /// Returns the SliderPane in the LabeledSliderPane.
    SliderPanePtr GetSliderPane() const;

  protected:
    LabeledSliderPane() {}

    virtual void AddFields() override;
    virtual bool IsValid(std::string &details) override;
    virtual void CreationDone() override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<Vector2f> range_{"range", {0, 1}};
    Parser::TField<float>    precision_{"precision", 0};
    Parser::TField<Color>    text_color_{"text_color", Color::Black()};
    ///@}

    TextPanePtr cur_text_pane_;

    /// This is called when the slider value in the SliderPane changes; it
    /// updates the text.
    void UpdateText_(float new_value);

    friend class Parser::Registry;
};

typedef std::shared_ptr<LabeledSliderPane> LabeledSliderPanePtr;
