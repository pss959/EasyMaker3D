#pragma once

#include <concepts>

#include "Base/Memory.h"
#include "Panes/Pane.h"
#include "Panels/Panel.h"
#include "Tests/SceneTestBase.h"
#include "Util/Assert.h"

DECL_SHARED_PTR(ButtonPane);
DECL_SHARED_PTR(CheckboxPane);
DECL_SHARED_PTR(DropdownPane);
DECL_SHARED_PTR(RadioButtonPane);
DECL_SHARED_PTR(SliderPane);
DECL_SHARED_PTR(TextInputPane);

/// Base class for Panel tests; it provides some conveniences to simplify and
/// clarify tests.
///
/// \ingroup Tests
class PanelTestBase : public SceneTestBase {
  protected:
    /// The constructor sets up the Panel::Context.
    PanelTestBase();
    virtual ~PanelTestBase();

    /// Sets a flag in the TestBoardAgent that prints useful information about
    /// Panel status changes to help with debugging.
    void SetPanelDebugFlag(bool b);

    /// Sets up to test a Panel of the templated type and name. This sets up a
    /// test Context in the Panel and stores it for later use. The derived
    /// Panel is returned.
    template <typename T> std::shared_ptr<T> InitPanel(const Str &type_name) {
        static_assert(std::derived_from<T, Panel> == true);
        auto panel = ReadRealNode<T>(GetContentsString_(), type_name);
        StorePanelWithContext_(panel);
        return panel;
    }

    /// Returns the test Context created for the Panel.
    Panel::Context & GetContext() { return *test_context_; }

    /// \name Pane access helpers
    ///@{

    /// Finds and returns the sub-pane in the Panel with the given name.
    /// Asserts if not found.
    PanePtr FindPane(const Str &name) {
        auto panel = GetCurrentPanel();
        ASSERT(panel);
        return panel->GetPane()->FindSubPane(name);
    }

    /// Finds and returns the sub-pane in the Panel with the given name and
    /// type. Asserts if not found.
    template <typename T>
    std::shared_ptr<T> FindTypedPane(const Str &name) {
        auto panel = GetCurrentPanel();
        ASSERT(panel);
        return panel->GetPane()->FindTypedSubPane<T>(name);
    }

    ///@}

    /// \name Pane interaction helpers
    /// Each of these simulates some sort of interaction with a specific type
    /// of Pane indicated by name in the currently-open Panel.
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

    /// Returns the result of the last call to Close() on the Panel and resets
    /// to empty.
    Str GetCloseResult();

    ///@}

    /// Returns the currently-open Panel.
    PanelPtr GetCurrentPanel();

  private:
    DECL_SHARED_PTR(TestBoardAgent);
    DECL_SHARED_PTR(TestSettingsAgent);

    Panel::ContextPtr    test_context_;
    TestBoardAgentPtr    test_board_agent_;
    TestSettingsAgentPtr test_settings_agent_;

    static Str        GetContentsString_();
    void              StorePanelWithContext_(const PanelPtr &panel);
};
