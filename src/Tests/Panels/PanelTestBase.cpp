#include "Tests/Panels/PanelTestBase.h"

#include "Agents/BoardAgent.h"
#include "Panes/ButtonPane.h"
#include "Panes/CheckboxPane.h"
#include "Panes/ContainerPane.h"
#include "Place/ClickInfo.h"
#include "SG/Search.h"
#include "Util/Assert.h"
#include "Util/String.h"
#include "Widgets/PushButtonWidget.h"

/// Derived BoardAgent that saves the result of calling Close() on a Panel. An
/// instance of this is set in the test context.
class PanelTestBase::TestBoardAgent : public BoardAgent {
  public:
    Panel *cur_panel = nullptr;  ///< Currently-open Panel.
    Str    close_result;         ///< Result passed to ClosePanel().

    virtual PanelPtr GetPanel(const Str &name) const override {
        ASSERTM(false, "TestBoardAgent::GetPanel called for " + name);
    }

    virtual void ClosePanel(const Str &result) override {
        ASSERT(cur_panel);
        cur_panel->SetIsShown(false);
        close_result = result;
        cur_panel = nullptr;
    }

    virtual void PushPanel(const PanelPtr &panel,
                           const ResultFunc &result_func) override {
        ASSERTM(false, "TestBoardAgent::PushPanel called for " +
                panel->GetDesc());
    }
};

PanelTestBase::PanelTestBase() : test_board_agent_(new TestBoardAgent) {}
PanelTestBase::~PanelTestBase() {}

PanePtr PanelTestBase::FindPane(const Panel &panel, const Str &name) {
    return SG::FindTypedNodeUnderNode<Pane>(*panel.GetPane(), name);
}

void PanelTestBase::ClickButtonPane(const Panel &panel, const Str &name) {
    auto but_pane = FindTypedPane<ButtonPane>(panel, name);
    ClickInfo info;  // Contents do not matter.
    but_pane->GetButton().Click(info);

}

void PanelTestBase::ToggleCheckboxPane(const Panel &panel, const Str &name) {
    auto cbox_pane = FindTypedPane<CheckboxPane>(panel, name);
    ClickInfo info;  // Contents do not matter.
    cbox_pane->GetActivationWidget()->Click(info);
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

void PanelTestBase::SetTestContext_(Panel &panel) {
    Panel::ContextPtr pc(new Panel::Context);
    pc->board_agent = test_board_agent_;
    panel.SetTestContext(pc);
    test_board_agent_->cur_panel = &panel;
}

