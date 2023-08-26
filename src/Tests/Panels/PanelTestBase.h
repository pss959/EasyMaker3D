#pragma once

#include <concepts>

#include "Base/Memory.h"
#include "Panes/ContainerPane.h"
#include "Panels/Panel.h"
#include "Tests/SceneTestBase.h"
#include "Tests/UnitTestTypeChanger.h"
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
    /// The constructor is passed a flag indicating whether there are any
    /// text-based Panes that need fonts set up (which slows things down).
    explicit PanelTestBase(bool need_text = false);
    virtual ~PanelTestBase();

    /// Sets up to test a Panel of the templated type and name. This sets up a
    /// test Context in the Panel and stores it for later use. The derived
    /// Panel is returned.
    template <typename T> std::shared_ptr<T> InitPanel(const Str &type_name) {
        UnitTestTypeChanger uttc(need_text_ ? Util::AppType::kInteractive :
                                 Util::AppType::kUnitTest);
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
        ASSERT(panel_);
        return panel_->GetPane()->FindPane(name);
    }

    /// Finds and returns the sub-pane in the Panel with the given name and
    /// type. Asserts if not found.
    template <typename T>
    std::shared_ptr<T> FindTypedPane(const Str &name) {
        ASSERT(panel_);
        return panel_->GetPane()->FindTypedPane<T>(name);
    }

    ///@}

    /// \name Pane interaction helpers
    ///@{

    /// Convenience that returns true if the ButtonPane with the given name in
    /// the Panel has interaction enabled.
    bool IsButtonPaneEnabled(const Str &name);

    /// Convenience that finds a ButtonPane with the given name in the Panel
    /// and simulates a click on it. Returns the ButtonPane.
    ButtonPanePtr ClickButtonPane(const Str &name);

    /// Convenience that finds a CheckboxPane with the given name in the Panel
    /// and toggles it. Returns the CheckboxPane.
    CheckboxPanePtr ToggleCheckboxPane(const Str &name);

    /// Convenience that finds a DropdownPane with the given name in the Panel
    /// and sets its choice to the given string. Returns the DropdownPane.
    DropdownPanePtr ChangeDropdownChoice(const Str &name, const Str &choice);

    /// Convenience that finds a RadioButtonPane with the given name in the
    /// Panel and sets its state to true. Returns the RadioButtonPane.
    RadioButtonPanePtr ActivateRadioButtonPane(const Str &name);

    /// Convenience that finds a LabeledSliderPane with the given name in the
    /// Panel and performs a mouse drag by the given vector. Returns the
    /// SliderPane inside the LabeledSliderPane.
    SliderPanePtr DragSlider(const Str &name, const Vector2f &vec);

    /// Convenience that finds a TextInputPane with the given name in the Panel
    /// and sets it to contain the given text string. Returns the
    /// TextInputPane.
    TextInputPanePtr SetTextInput(const Str &name, const Str &text);

    /// Returns the result of the last call to Close() on the Panel and resets
    /// to empty.
    Str GetCloseResult();

    ///@}

  private:
    DECL_SHARED_PTR(TestBoardAgent);

    const bool        need_text_ = false;
    PanelPtr          panel_;
    Panel::ContextPtr test_context_;
    TestBoardAgentPtr test_board_agent_;

    static Str        GetContentsString_();
    void              StorePanelWithContext_(const PanelPtr &panel);
};
