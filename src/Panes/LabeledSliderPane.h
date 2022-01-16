#pragma once

#include <memory>

#include "Math/Types.h"
#include "Panes/BoxPane.h"
#include "Panes/TextPane.h"

namespace Parser { class Registry; }

/// LabeledSliderPane is a derived BoxPane that adds text labels to a
/// SliderPane to (optionally) show the minimum, maximum, and current values.
class LabeledSliderPane : public BoxPane {
  protected:
    LabeledSliderPane() {}

    virtual void AddFields() override;
    virtual bool IsValid(std::string &details) override;
    virtual void CreationDone(bool is_template) override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<Vector2f> range_{"range", {0, 1}};
    Parser::TField<float>    precision_{"precision", 0};
    Parser::TField<bool>     show_min_{"show_min", true};
    Parser::TField<bool>     show_max_{"show_max", true};
    Parser::TField<bool>     show_current_{"show_current", true};
    Parser::TField<Color>    text_color_{"text_color", Color::Black()};
    ///@}

    TextPanePtr min_text_pane_;
    TextPanePtr max_text_pane_;
    TextPanePtr cur_text_pane_;

    /// This is called when the slider value in the SliderPane changes; it
    /// updates the text.
    void UpdateText_(float new_value);

    friend class Parser::Registry;
};

typedef std::shared_ptr<LabeledSliderPane> LabeledSliderPanePtr;
