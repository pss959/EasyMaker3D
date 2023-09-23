#include "Tests/Panels/PanelTestBase.h"

#include <ranges>
#include <vector>

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
    // If this flag is set, messages are printed to help debug Panel issues.
    bool debug_panels = false;

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
        return panel_stack_.back().panel;
    }

    /// Returns the last result passed to ClosePanel(), then clears it.
    Str GetCloseResult();

  private:
    struct PanelInfo_ {
        PanelPtr               panel;
        BoardAgent::ResultFunc result_func;
    };

    SG::ScenePtr            scene_;         ///< Used to find other Panels.
    Panel::ContextPtr       context_;       ///< Context for other panels.
    std::vector<PanelInfo_> panel_stack_;   ///< Stack of currently-open Panels.
    Str                     close_result_;  ///< Last result of ClosePanel().
    size_t                  nesting_ = 0;   ///< Level of nesting of operations.
    bool                    is_first_panel_ = true;

    /// For indenting according to current nesting_ level.
    Str Indent_() const {
        return "== [" + Util::ToString(nesting_) + "]" +
            Util::Spaces(2 * nesting_);
    }

    /// Prints the current Panel stack for debugging help.
    void PrintStack_(const Str &when) const;
};

PanelPtr PanelTestBase::TestBoardAgent::GetPanel(const Str &name) const {
    ASSERT(scene_);
    auto panel = SG::FindTypedNodeInScene<Panel>(*scene_, name);

    // Make sure it has a context.
    panel->SetTestContext(context_);
    return panel;
}

void PanelTestBase::TestBoardAgent::ClosePanel(const Str &result) {
    ASSERT(! panel_stack_.empty());

    // Copy PanelInfo_ to protect after pop().
    const auto info = panel_stack_.back();
    panel_stack_.pop_back();

    if (debug_panels) {
        std::cerr << Indent_() << "Closing " << info.panel->GetDesc()
                  << " with result '" << result << "'\n";
        std::cerr << Indent_() << "Hiding  " << info.panel->GetDesc() << "\n";
        PrintStack_("after pop");
    }

    info.panel->SetStatus(Panel::Status::kUnattached);

    if (! panel_stack_.empty()) {
        const auto &new_panel = panel_stack_.back().panel;
        if (debug_panels)
            std::cerr << Indent_() << " Showing "
                      << new_panel->GetDesc() << "\n";
        new_panel->SetStatus(Panel::Status::kVisible);
    }

    if (info.result_func) {
        if (debug_panels)
            std::cerr << Indent_() << " Invoking result func for "
                      << info.panel->GetDesc() << "\n";
        info.result_func(result);
    }

    close_result_ = result;

    if (debug_panels)
        PrintStack_("after ClosePanel for " + info.panel->GetName());

    ASSERT(nesting_ > 0U);
    --nesting_;
}

void PanelTestBase::TestBoardAgent::PushPanel(const PanelPtr &panel,
                                              const ResultFunc &result_func) {
    if (debug_panels)
        std::cerr << Indent_() << "Pushing " << panel->GetDesc() << "\n";

    // Hide the previous panel if any.
    if (! panel_stack_.empty()) {
        if (debug_panels)
            std::cerr << Indent_() << "Hiding  "
                      << panel_stack_.back().panel->GetDesc() << "\n";
        panel_stack_.back().panel->SetStatus(Panel::Status::kHidden);
    }

    // Push a PanelInfo_ for the new Panel.
    PanelInfo_ info;
    info.panel       = panel;
    info.result_func = result_func;
    panel_stack_.push_back(info);

    // Set its context and make sure it has a valid size (needed for
    // TextInputPanes).
    panel->SetTestContext(context_);
    panel->SetSize(panel->GetPane()->GetBaseSize());

    // Do NOT show the Panel if it is the first one - that allows tests to do
    // that for the first time.
    if (! is_first_panel_) {
        if (debug_panels)
            std::cerr << Indent_() << " Showing " << panel->GetDesc() << "\n";
        panel->SetStatus(Panel::Status::kVisible);
    }
    if (debug_panels)
        PrintStack_("after PushPanel for " + panel->GetName());

    ++nesting_;
    is_first_panel_ = false;
}

Str PanelTestBase::TestBoardAgent::GetCloseResult() {
    const auto result = close_result_;
    close_result_.clear();
    return result;
}

void PanelTestBase::TestBoardAgent::PrintStack_(const Str &when) const {
    std::cerr << "....... Top-down Panel stack " << when << ":\n";
    for (const auto &info: panel_stack_ | std::views::reverse) {
        std::cerr << "........    " << info.panel->GetDesc()
                  << "(" << Util::EnumToWord(info.panel->GetStatus()) << ")\n";
    }
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

PanelTestBase::PanelTestBase() {
    test_board_agent_.reset(new TestBoardAgent);
    test_settings_agent_.reset(new TestSettingsAgent);

    // Create and store a test Context.
    test_context_.reset(new Panel::Context);
    test_context_->board_agent = test_board_agent_;
    test_context_->name_agent.reset(new NameManager);
    test_context_->settings_agent = test_settings_agent_;
    test_context_->virtual_keyboard.reset(new VirtualKeyboard);
}

PanelTestBase::~PanelTestBase() {
    // Reset pointers so instances are freed up. (There are some circular
    // dependencies.)
    test_context_->board_agent.reset();
    test_context_->name_agent.reset();
    test_context_->settings_agent.reset();
    test_context_.reset();
    test_board_agent_.reset();
    test_settings_agent_.reset();
}

void PanelTestBase::SetPanelDebugFlag(bool b) {
    test_board_agent_->debug_panels = true;
}

void PanelTestBase::StoreContext() {
    // Tell the TestBoardAgent where to find other panels and the context to
    // use for them.
    test_board_agent_->SetScene(GetScene());
    test_board_agent_->SetContext(test_context_);
}

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

Str PanelTestBase::GetContentsString_(const Str &extra_contents) {
    return R"(
  children: [
    <"nodes/templates/RadialMenu.emd">, # Required for RadialMenuPanel
    <"nodes/Panels.emd">,)" + extra_contents + R"(
  ]
)";
}

void PanelTestBase::StorePanelWithContext_(const PanelPtr &panel) {
    StoreContext();
    test_board_agent_->PushPanel(panel, nullptr);
}
