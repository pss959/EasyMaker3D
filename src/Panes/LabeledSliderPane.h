#pragma once

#include "Base/Memory.h"
#include "Math/Types.h"
#include "Panes/BoxPane.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(LabeledSliderPane);
DECL_SHARED_PTR(SliderPane);
DECL_SHARED_PTR(TextPane);

/// LabeledSliderPane is a derived BoxPane that adds text labels to a
/// SliderPane to show the minimum, maximum, and current values.
///
/// \ingroup Panes
class LabeledSliderPane : public BoxPane {
  public:
    /// Returns the SliderPane in the LabeledSliderPane.
    SliderPanePtr GetSliderPane() const;

  protected:
    LabeledSliderPane() {}

    virtual void AddFields() override;
    virtual bool IsValid(Str &details) override;
    virtual void CreationDone() override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<Vector2f> range_;
    Parser::TField<float>    precision_;
    Parser::TField<Color>    text_color_;
    ///@}

    TextPanePtr cur_text_pane_;

    /// This is called when the slider value in the SliderPane changes; it
    /// updates the text.
    void UpdateText_(float new_value);

    friend class Parser::Registry;
};
