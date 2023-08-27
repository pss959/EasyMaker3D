#include "Tests/Panels/PanelTestBase.h"

#include <stack>

#include "Agents/BoardAgent.h"
#include "Agents/SettingsAgent.h"
#include "Base/VirtualKeyboard.h"
#include "Items/Settings.h"
#include "Managers/NameManager.h"
#include "Panes/ButtonPane.h"
#include "Panes/CheckboxPane.h"
#include "Panes/ContainerPane.h"
#include "Panes/DropdownPane.h"
#include "Panes/LabeledSliderPane.h"
#include "Panes/RadioButtonPane.h"
#include "Panes/SliderPane.h"
#include "Panes/TextInputPane.h"
#include "Place/ClickInfo.h"
#include "SG/Search.h"
#include "Tests/Widgets/DragTester.h"
#include "Util/Enum.h"
#include "Util/String.h"
#include "Widgets/PushButtonWidget.h"
#include "Widgets/Slider1DWidget.h"

// ----------------------------------------------------------------------------
// PanelTestBase::TestBoardAgent class.
// ----------------------------------------------------------------------------

/// Derived BoardAgent that saves a stack of Panels and the results of calling
/// Close() on them. An instance of this is set in the test context.
///
/// \ingroup Tests
class PanelTestBase::TestBoardAgent : public BoardAgent {
  public:
    /// Sets the Scene to use to find other Panels.
    void SetScene(const SG::ScenePtr &scene) { scene_ = scene; }
    /// Sets the Panel::Context to use for other Panels.
    void SetContext(const Panel::ContextPtr &context) { context_ = context; }

    virtual PanelPtr GetPanel(const Str &name) const override;
    virtual void ClosePanel(const Str &result) override;
    virtual void PushPanel(const PanelPtr &panel,
                           const ResultFunc &result_func) override;

    /// Returns the currently-open panel
    PanelPtr GetCurrentPanel() {
        ASSERT(! panel_stack_.empty());
        return panel_stack_.top().panel;
    }

    /// Returns the last result passed to ClosePanel(), then clears it.
    Str GetCloseResult();

  private:
    struct PanelInfo_ {
        PanelPtr               panel;
        BoardAgent::ResultFunc result_func;
    };

    SG::ScenePtr           scene_;         ///< Used to find other Panels.
    Panel::ContextPtr      context_;       ///< Context for other panels.
    std::stack<PanelInfo_> panel_stack_;   ///< Stack of currently-open Panels.
    Str                    close_result_;  ///< Last result of ClosePanel().
};

PanelPtr PanelTestBase::TestBoardAgent::GetPanel(const Str &name) const {
    ASSERT(scene_);
    return SG::FindTypedNodeInScene<Panel>(*scene_, name);
}

void PanelTestBase::TestBoardAgent::ClosePanel(const Str &result) {
    ASSERT(! panel_stack_.empty());
    const auto &info = panel_stack_.top();
    info.panel->SetIsShown(false);

    if (info.result_func)
        info.result_func(result);

    panel_stack_.pop();
    if (! panel_stack_.empty())
        panel_stack_.top().panel->SetIsShown(true);

    close_result_ = result;
}

void PanelTestBase::TestBoardAgent::PushPanel(const PanelPtr &panel,
                                              const ResultFunc &result_func) {
    const bool is_first_panel = panel_stack_.empty();

    // Hide the previous panel if any.
    if (! is_first_panel)
        panel_stack_.top().panel->SetIsShown(false);

    // Push a PanelInfo_ for the new Panel.
    PanelInfo_ info;
    info.panel       = panel;
    info.result_func = result_func;
    panel_stack_.push(info);

    // Set its context.
    panel->SetTestContext(context_);

    // Do NOT show the Panel if it is the first one - that allows tests to do
    // that for the first time.
    if (! is_first_panel)
        panel->SetIsShown(true);
}

Str PanelTestBase::TestBoardAgent::GetCloseResult() {
    const auto result = close_result_;
    close_result_.clear();
    return result;
}

// ----------------------------------------------------------------------------
// PanelTestBase::TestSettingsAgent class.
// ----------------------------------------------------------------------------

/// Derived SettingsAgent that returns a consistent Settings instance for
/// GetSettings(). An instance of this is set in the test context.
///
/// \ingroup Tests
class PanelTestBase::TestSettingsAgent : public SettingsAgent {
  public:
    TestSettingsAgent();
    virtual const Settings & GetSettings() const override {
        return *settings_;
    }
    virtual void SetSettings(const Settings &new_settings) {
        settings_->CopyFrom(new_settings);
    }
  private:
    SettingsPtr settings_;
};

PanelTestBase::TestSettingsAgent::TestSettingsAgent() {
    auto imconv = UnitConversion::CreateWithUnits(
        UnitConversion::Units::kFeet, UnitConversion::Units::kInches);
    auto exconv = UnitConversion::CreateWithUnits(
        UnitConversion::Units::kInches, UnitConversion::Units::kFeet);

    auto linfo = ParseTypedObject<RadialMenuInfo>(
        R"(RadialMenuInfo { count: "kCount4",
            actions: [ "kDelete", "kCut", "kCopy", "kPaste" ] })");
    auto rinfo = ParseTypedObject<RadialMenuInfo>(
        R"(RadialMenuInfo { count: "kCount8",
            actions: [ "kCreateBox", "kCreateCylinder", "kCreateExtruded",
                       "kCreateImportedModel", "kCreateRevSurf",
                       "kCreateSphere", "kCreateText", "kCreateTorus" ] })");

    settings_ = Settings::CreateDefault();
    settings_->SetLastSessionPath("/a/b/c");
    settings_->SetSessionDirectory("/d/e/f");
    settings_->SetImportDirectory("/g/h/i");
    settings_->SetExportDirectory("/j/k/l");
    settings_->SetTooltipDelay(1.5f);
    settings_->SetImportUnitsConversion(*imconv);
    settings_->SetExportUnitsConversion(*exconv);
    settings_->SetBuildVolumeSize(Vector3f(3, 4, 5));
    settings_->SetLeftRadialMenuInfo(*linfo);
    settings_->SetRightRadialMenuInfo(*rinfo);
    settings_->SetRadialMenusMode(RadialMenusMode::kIndependent);
}

// ----------------------------------------------------------------------------
// PanelTestBase functions.
// ----------------------------------------------------------------------------

PanelTestBase::PanelTestBase(bool need_text) : need_text_(need_text) {
    test_board_agent_.reset(new TestBoardAgent);
    test_settings_agent_.reset(new TestSettingsAgent);

    // Create and store a test Context.
    test_context_.reset(new Panel::Context);
    test_context_->board_agent = test_board_agent_;
    test_context_->name_agent.reset(new NameManager);
    test_context_->settings_agent = test_settings_agent_;
    test_context_->virtual_keyboard.reset(new VirtualKeyboard);
}

PanelTestBase::~PanelTestBase() {}

bool PanelTestBase::IsButtonPaneEnabled(const Str &name) {
    return FindTypedPane<ButtonPane>(name)->IsInteractionEnabled();
}

ButtonPanePtr PanelTestBase::ClickButtonPane(const Str &name) {
    auto but_pane = FindTypedPane<ButtonPane>(name);
    ClickInfo info;  // Contents do not matter.
    but_pane->GetButton().Click(info);
    return but_pane;
}

CheckboxPanePtr PanelTestBase::ToggleCheckboxPane(const Str &name) {
    auto cbox_pane = FindTypedPane<CheckboxPane>(name);
    ClickInfo info;  // Contents do not matter.
    cbox_pane->GetActivationWidget()->Click(info);
    return cbox_pane;
}

DropdownPanePtr PanelTestBase::ChangeDropdownChoice(const Str &name,
                                                    const Str &choice) {
    auto dd_pane = FindTypedPane<DropdownPane>(name);
    dd_pane->SetChoiceFromString(choice, true);  // Notify = true.
    return dd_pane;
}

RadioButtonPanePtr PanelTestBase::ActivateRadioButtonPane(const Str &name) {
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
    auto input_pane = FindTypedPane<TextInputPane>(name);
    input_pane->GetInteractor()->Activate();
    input_pane->SetInitialText(text);
    input_pane->GetInteractor()->Deactivate();
    return input_pane;
}

Str PanelTestBase::GetCloseResult() {
    return test_board_agent_->GetCloseResult();
}

PanelPtr PanelTestBase::GetCurrentPanel() {
    return test_board_agent_->GetCurrentPanel();
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
    // Tell the TestBoardAgent where to find other panels and the context to
    // use for them.
    test_board_agent_->SetScene(GetScene());
    test_board_agent_->SetContext(test_context_);

    test_board_agent_->PushPanel(panel, nullptr);
}
