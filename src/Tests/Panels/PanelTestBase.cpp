#include "Tests/Panels/PanelTestBase.h"

#include "Agents/BoardAgent.h"
#include "Managers/NameManager.h"
#include "Panes/ButtonPane.h"
#include "Panes/CheckboxPane.h"
#include "Panes/ContainerPane.h"
#include "Panes/LabeledSliderPane.h"
#include "Panes/RadioButtonPane.h"
#include "Panes/SliderPane.h"
#include "Panes/TextInputPane.h"
#include "Place/ClickInfo.h"
#include "SG/Search.h"
#include "Tests/Widgets/DragTester.h"
#include "Util/Assert.h"
#include "Util/Enum.h"
#include "Util/String.h"
#include "Widgets/PushButtonWidget.h"
#include "Widgets/Slider1DWidget.h"

// ----------------------------------------------------------------------------
// PanelTestBase::TestBoardAgent class.
// ----------------------------------------------------------------------------

/// Derived BoardAgent that saves the result of calling Close() on a Panel. An
/// instance of this is set in the test context.
///
/// \ingroup Tests
class PanelTestBase::TestBoardAgent : public BoardAgent {
  public:
    PanelPtr cur_panel;     ///< Currently-open Panel.
    Str      close_result;  ///< Result passed to ClosePanel().

    virtual PanelPtr GetPanel(const Str &name) const override {
        ASSERTM(false, "TestBoardAgent::GetPanel called for " + name);
    }

    virtual void ClosePanel(const Str &result) override {
        ASSERT(cur_panel);
        cur_panel->SetIsShown(false);
        close_result = result;
        cur_panel.reset();
    }

    virtual void PushPanel(const PanelPtr &panel,
                           const ResultFunc &result_func) override {
        ASSERTM(false, "TestBoardAgent::PushPanel called for " +
                panel->GetDesc());
    }
};

// ----------------------------------------------------------------------------
// PanelTestBase functions.
// ----------------------------------------------------------------------------

PanelTestBase::PanelTestBase(bool need_text) : need_text_(need_text) {
    test_board_agent_.reset(new TestBoardAgent);

    // Create and store a test Context.
    test_context_.reset(new Panel::Context);
    test_context_->board_agent = test_board_agent_;
    test_context_->name_agent.reset(new NameManager);
}

PanelTestBase::~PanelTestBase() {}

PanePtr PanelTestBase::FindPane(const Str &name) {
    ASSERT(panel_);
    return SG::FindTypedNodeUnderNode<Pane>(*panel_->GetPane(), name);
}

ButtonPanePtr PanelTestBase::ClickButtonPane(const Str &name) {
    ASSERT(panel_);
    auto but_pane = FindTypedPane<ButtonPane>(name);
    ClickInfo info;  // Contents do not matter.
    but_pane->GetButton().Click(info);
    return but_pane;
}

CheckboxPanePtr PanelTestBase::ToggleCheckboxPane(const Str &name) {
    ASSERT(panel_);
    auto cbox_pane = FindTypedPane<CheckboxPane>(name);
    ClickInfo info;  // Contents do not matter.
    cbox_pane->GetActivationWidget()->Click(info);
    return cbox_pane;
}

RadioButtonPanePtr PanelTestBase::ActivateRadioButtonPane(const Str &name) {
    ASSERT(panel_);
    auto rbut_pane = FindTypedPane<RadioButtonPane>(name);
    rbut_pane->SetState(true);
    return rbut_pane;
}

SliderPanePtr PanelTestBase::DragSlider(const Str &name, const Vector2f &vec) {
    auto lsp = FindTypedPane<LabeledSliderPane>(name)->GetSliderPane();
    auto s1w =
        std::dynamic_pointer_cast<Slider1DWidget>(lsp->GetActivationWidget());

    DragTester dt(s1w);
    dt.SetRayDirection(-Vector3f::AxisZ());
    dt.ApplyMouseDrag(Point3f(0, 0, 0), Point3f(vec[0], vec[1], 0));
    return lsp;
}

TextInputPanePtr PanelTestBase::SetTextInput(const Str &name, const Str &text) {
    ASSERT(panel_);
    auto input_pane = FindTypedPane<TextInputPane>(name);
    input_pane->GetInteractor()->Activate();
    input_pane->SetInitialText(text);
    input_pane->GetInteractor()->Deactivate();
    return input_pane;
}

Str PanelTestBase::GetCloseResult() {
    const Str result = test_board_agent_->close_result;
    test_board_agent_->close_result.clear();
    return result;
}

Str PanelTestBase::GetContentsString_() {
    return R"(
  children: [
    <"nodes/templates/RadialMenu.emd">, # Required for RadialMenuPanel
    <"nodes/Panels.emd">,
  ]
)";
}

void PanelTestBase::StorePanelWithContext_(const PanelPtr &panel) {
    panel_ = panel;
    panel_->SetTestContext(test_context_);
    test_board_agent_->cur_panel = panel;
}
