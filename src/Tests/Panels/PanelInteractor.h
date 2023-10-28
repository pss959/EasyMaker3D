#pragma once

#include <concepts>
#include <functional>

#include "Panels/Panel.h"
#include "Panes/Pane.h"
#include "Util/Assert.h"
#include "Util/Memory.h"

DECL_SHARED_PTR(ButtonPane);
DECL_SHARED_PTR(CheckboxPane);
DECL_SHARED_PTR(DropdownPane);
DECL_SHARED_PTR(RadioButtonPane);
DECL_SHARED_PTR(SliderPane);
DECL_SHARED_PTR(TextInputPane);

/// This class can be used to help test interaction with a Panel.
///
/// \ingroup Tests
class PanelInteractor {
  public:
    /// Constructor that takes a Panel to interact with.
    explicit PanelInteractor(const PanelPtr &panel);

    /// Constructor that takes a function to call to get the Panel to interact
    /// with.
    explicit PanelInteractor(const std::function<PanelPtr()> &panel_func);

    /// \name Pane access helpers
    ///@{

    /// Finds and returns the sub-pane in the Panel with the given name and
    /// type. Asserts if not found.
    template <typename T>
    std::shared_ptr<T> FindTypedPane(const Str &name) {
        auto panel = GetPanel_();
        ASSERT(panel);
        return panel->GetPane()->FindTypedSubPane<T>(name);
    }

    ///@}

    /// \name Pane interaction helpers
    /// Each of these simulates some sort of interaction with a specific type
    /// of Pane indicated by name in the Panel.
    ///@{

    /// Convenience that returns true if the named ButtonPane has interaction
    /// enabled.
    bool IsButtonPaneEnabled(const Str &name);

    /// Convenience that simulates a click on the named ButtonPane. Returns the
    /// ButtonPane.
    ButtonPanePtr ClickButtonPane(const Str &name);

    /// Convenience that toggles the named CheckboxPane. Returns the
    /// CheckboxPane.
    CheckboxPanePtr ToggleCheckboxPane(const Str &name);

    /// Convenience that sets the choice (by string) in the named DropdownPane.
    /// Returns the DropdownPane.
    DropdownPanePtr ChangeDropdownChoice(const Str &name, const Str &choice);

    /// Convenience that sets the named RadioButtonPane to true with
    /// notification turned on. Returns the RadioButtonPane.
    RadioButtonPanePtr ActivateRadioButtonPane(const Str &name);

    /// Convenience that performs a mouse drag on the named LabeledSliderPane.
    /// Returns the SliderPane inside the LabeledSliderPane.
    SliderPanePtr DragSlider(const Str &name, const Vector2f &vec);

    /// Convenience that sets the text string in the named TextInputPane.
    /// Returns the TextInputPane.
    TextInputPanePtr SetTextInput(const Str &name, const Str &text);

    ///@}

  private:
    PanelPtr                  panel_;
    std::function<PanelPtr()> panel_func_;

    /// Returns the Panel to interact with.
    PanelPtr GetPanel_() { return panel_ ? panel_ : panel_func_(); }
};
