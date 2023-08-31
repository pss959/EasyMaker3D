#include "Agents/SessionAgent.h"
#include "Base/Event.h"
#include "Panels/SessionPanel.h"
#include "Panes/ButtonPane.h"
#include "Panes/TextPane.h"
#include "Tests/Panels/PanelTestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class SessionPanelTest : public PanelTestBase {
  protected:
    /// Derived SessionAgent that allows for simulation of various session
    /// conditions for testing the SessionPanel.
    class TestSessionAgent : public SessionAgent {
      public:
        bool can_save  = false;  ///< Whether a session can be saved.
        bool save_ok   = true;   ///< Value for SaveSession() to return.
        bool load_ok   = true;   ///< Value for LoadSession() to return.
        bool export_ok = true;   ///< Value for Export() to return.
        Str  prev_name;          ///< Name of previous session.
        Str  cur_name;           ///< Name of current session.
        Str  export_name;        ///< Model name to use for export.
        Str  last_op;            ///< Names last operation performed.

        virtual void NewSession() override;
        virtual bool CanSaveSession() const override;
        virtual bool SaveSession(const FilePath &) override;
        virtual bool LoadSession(const FilePath &, Str &) override;
        virtual Str  GetModelNameForExport() const override;
        virtual bool Export(const FilePath &, FileFormat,
                            const UnitConversion &) override;
        virtual const Str & GetPreviousSessionName() const override;
        virtual const Str & GetCurrentSessionName() const override;
    };
    DECL_SHARED_PTR(TestSessionAgent);

    SessionPanelPtr     panel;
    TestSessionAgentPtr session_agent;

    SessionPanelTest() : session_agent(new TestSessionAgent) {
        GetContext().session_agent = session_agent;
        panel = InitPanel<SessionPanel>("SessionPanel");
    }
};

// ----------------------------------------------------------------------------
// SessionPanelTest::TestSessionAgent functions.
// ----------------------------------------------------------------------------

void SessionPanelTest::TestSessionAgent::NewSession() {
    last_op = "New";
}

bool SessionPanelTest::TestSessionAgent::CanSaveSession() const {
    return can_save;
}

bool SessionPanelTest::TestSessionAgent::SaveSession(const FilePath &p) {
    last_op = "Save";
    return save_ok;
}

bool SessionPanelTest::TestSessionAgent::LoadSession(const FilePath &, Str &) {
    last_op = "Load";
    return load_ok;
}

Str SessionPanelTest::TestSessionAgent::GetModelNameForExport() const {
    return export_name;
}

bool SessionPanelTest::TestSessionAgent::Export(const FilePath &, FileFormat,
                                                const UnitConversion &) {
    last_op = "Export";
    return export_ok;
}

const Str & SessionPanelTest::TestSessionAgent::GetPreviousSessionName() const {
    return prev_name;
}

const Str & SessionPanelTest::TestSessionAgent::GetCurrentSessionName() const {
    return cur_name;
}

// ----------------------------------------------------------------------------
// SessionPanel tests.
// ----------------------------------------------------------------------------

TEST_F(SessionPanelTest, Defaults) {
    EXPECT_NULL(panel->GetFocusedPane());
}

TEST_F(SessionPanelTest, Show) {
    EXPECT_FALSE(panel->IsShown());
    panel->SetIsShown(true);
    EXPECT_TRUE(panel->IsShown());
}

TEST_F(SessionPanelTest, ButtonState) {
    // Test button state under various initial conditions when showing Panel.
    auto &agent = *session_agent;
    auto cont = FindTypedPane<ButtonPane>("Continue");

    // No previous or current session names, no changes.
    agent.can_save    = false;
    agent.prev_name   = "";
    agent.cur_name    = "";
    agent.export_name = "";
    panel->SetIsShown(true);
    EXPECT_EQ("(No previous session)",
              cont->FindTypedPane<TextPane>("ButtonText")->GetText());
    EXPECT_FALSE(IsButtonPaneEnabled("Continue"));
    EXPECT_TRUE(IsButtonPaneEnabled("Load"));
    EXPECT_FALSE(IsButtonPaneEnabled("Save"));
    EXPECT_FALSE(IsButtonPaneEnabled("Export"));
    EXPECT_EQ(FindPane("New"), panel->GetFocusedPane());
    panel->SetIsShown(false);

    // Have previous but no current session name, no changes.
    agent.can_save    = false;
    agent.prev_name   = "Prev.ems";
    agent.cur_name    = "";
    agent.export_name = "";
    panel->SetIsShown(true);
    EXPECT_EQ("Continue previous session [Prev.ems]",
              cont->FindTypedPane<TextPane>("ButtonText")->GetText());
    EXPECT_TRUE(IsButtonPaneEnabled("Continue"));
    EXPECT_TRUE(IsButtonPaneEnabled("Load"));
    EXPECT_FALSE(IsButtonPaneEnabled("Save"));
    EXPECT_FALSE(IsButtonPaneEnabled("Export"));
    // Focus should not have changed.
    EXPECT_EQ(FindPane("New"), panel->GetFocusedPane());
    panel->SetIsShown(false);

    // Have current but no previous session name, scene has changes and there
    // is a Model to export.
    agent.can_save    = true;
    agent.prev_name   = "";
    agent.cur_name    = "Cur.ems";
    agent.export_name = "SomeModel";
    panel->SetIsShown(true);
    EXPECT_EQ("Continue current session [Cur.ems]",
              cont->FindTypedPane<TextPane>("ButtonText")->GetText());
    EXPECT_TRUE(IsButtonPaneEnabled("Continue"));
    EXPECT_TRUE(IsButtonPaneEnabled("Load"));
    EXPECT_TRUE(IsButtonPaneEnabled("Save"));
    EXPECT_TRUE(IsButtonPaneEnabled("Export"));
    // Focus should not have changed.
    EXPECT_EQ(FindPane("New"), panel->GetFocusedPane());
    panel->SetIsShown(false);

    // Have no current or previous session names, scene has changes.
    agent.can_save    = true;
    agent.prev_name   = "";
    agent.cur_name    = "";
    agent.export_name = "";
    panel->SetIsShown(true);
    EXPECT_EQ("Continue current session",
              cont->FindTypedPane<TextPane>("ButtonText")->GetText());
    EXPECT_TRUE(IsButtonPaneEnabled("Continue"));
    EXPECT_TRUE(IsButtonPaneEnabled("Load"));
    EXPECT_FALSE(IsButtonPaneEnabled("Save"));
    EXPECT_FALSE(IsButtonPaneEnabled("Export"));
    // Focus should not have changed.
    EXPECT_EQ(FindPane("New"), panel->GetFocusedPane());
    panel->SetIsShown(false);
}

TEST_F(SessionPanelTest, ButtonFocusContinue) {
    // Make sure focus is on "Continue" button if it is enabled at
    // start. (Focus is persistent, so have to test this with a new instance.
    auto &agent = *session_agent;
    auto cont = FindTypedPane<ButtonPane>("Continue");

    agent.can_save  = false;
    agent.prev_name = "Prev.ems";
    agent.cur_name  = "";
    panel->SetIsShown(true);
    EXPECT_EQ("Continue previous session [Prev.ems]",
              cont->FindTypedPane<TextPane>("ButtonText")->GetText());
    EXPECT_TRUE(IsButtonPaneEnabled("Continue"));
    EXPECT_TRUE(IsButtonPaneEnabled("Load"));
    EXPECT_FALSE(IsButtonPaneEnabled("Save"));
    EXPECT_FALSE(IsButtonPaneEnabled("Export"));
    EXPECT_EQ(FindPane("Continue"), panel->GetFocusedPane());
}

TEST_F(SessionPanelTest, OpenOtherPanels) {
    // Click the "Help" button: the SessionPanel should be hidden and the
    // HelpPanel should be shown.
    ClickButtonPane("Help");
    EXPECT_FALSE(panel->IsShown());
    auto help_panel = GetCurrentPanel();
    EXPECT_EQ("HelpPanel", help_panel->GetTypeName());
    EXPECT_TRUE(help_panel->IsShown());
    ClickButtonPane("Done");
    EXPECT_EQ("Done", GetCloseResult());
    EXPECT_FALSE(help_panel->IsShown());
    EXPECT_TRUE(panel->IsShown());

    // Repeat with the "Settings" button.
    ClickButtonPane("Settings");
    EXPECT_FALSE(panel->IsShown());
    auto settings_panel = GetCurrentPanel();
    EXPECT_EQ("SettingsPanel", settings_panel->GetTypeName());
    EXPECT_TRUE(settings_panel->IsShown());
    ClickButtonPane("Cancel");
    EXPECT_EQ("Cancel", GetCloseResult());
    EXPECT_FALSE(settings_panel->IsShown());
    EXPECT_TRUE(panel->IsShown());
}

TEST_F(SessionPanelTest, Continue) {
    // Session just started (no current session name, no changes) and the user
    // continued a previous session => load the previous session file.
    auto &agent = *session_agent;

    agent.can_save  = false;
    agent.prev_name = "Prev.ems";
    agent.cur_name  = "";
    panel->SetIsShown(true);

    // An unsuccessful load should show a DialogPanel with the error message.
    agent.load_ok = false;
    ClickButtonPane("Continue");
    EXPECT_EQ("Load", agent.last_op);
    EXPECT_FALSE(panel->IsShown());
    auto dialog = GetCurrentPanel();
    EXPECT_EQ("DialogPanel", dialog->GetTypeName());
    EXPECT_TRUE(dialog->IsShown());
    ClickButtonPane("Button0");
    EXPECT_EQ("OK", GetCloseResult());
    EXPECT_FALSE(dialog->IsShown());
    EXPECT_TRUE(panel->IsShown());

    // A successful load should close the SessionPanel.
    agent.load_ok = true;
    ClickButtonPane("Continue");
    EXPECT_EQ("Load", agent.last_op);
    EXPECT_FALSE(panel->IsShown());
    EXPECT_EQ("Done", GetCloseResult());
}

TEST_F(SessionPanelTest, Load) {
    auto &agent = *session_agent;

    panel->SetIsShown(true);

    // Force the "really load" Dialog to appear.
    agent.can_save = true;
    agent.save_ok  = true;

    // Clicking the Load button should show a FilePanel.
    ClickButtonPane("Load");
    EXPECT_FALSE(panel->IsShown());
    auto file_panel = GetCurrentPanel();
    EXPECT_EQ("FilePanel", file_panel->GetTypeName());
    EXPECT_TRUE(file_panel->IsShown());
    // This has to be a valid file for the Accept button to be enabled.
    SetTextInput("Input", GetDataPath("Sessions/Empty.ems").ToString());
    EXPECT_TRUE(IsButtonPaneEnabled("Accept"));

    // Accepting the file to load should show the DialogPanel.
    ClickButtonPane("Accept");
    EXPECT_EQ("Accept", GetCloseResult());
    EXPECT_FALSE(file_panel->IsShown());
    auto dialog = GetCurrentPanel();
    EXPECT_EQ("DialogPanel", dialog->GetTypeName());
    EXPECT_TRUE(dialog->IsShown());

    // Click the "Yes" button to really load the session and close the
    // SessionPanel.
    ClickButtonPane("Button1");  // "Yes".
    EXPECT_EQ("Yes", GetCloseResult());
    EXPECT_FALSE(dialog->IsShown());
    EXPECT_FALSE(panel->IsShown());
    EXPECT_EQ("Load", agent.last_op);
}

TEST_F(SessionPanelTest, LoadFail) {
    auto &agent = *session_agent;

    panel->SetIsShown(true);

    // Clicking the Load button should show a FilePanel.
    agent.load_ok = false;
    ClickButtonPane("Load");
    EXPECT_FALSE(panel->IsShown());
    auto file_panel = GetCurrentPanel();
    EXPECT_EQ("FilePanel", file_panel->GetTypeName());
    EXPECT_TRUE(file_panel->IsShown());
    // This has to be a valid file for the Accept button to be enabled.
    SetTextInput("Input", GetDataPath("Sessions/Empty.ems").ToString());
    EXPECT_TRUE(IsButtonPaneEnabled("Accept"));

    // Accepting the file to load should show the error DialogPanel.
    ClickButtonPane("Accept");
    EXPECT_EQ("Accept", GetCloseResult());
    EXPECT_FALSE(file_panel->IsShown());
    auto dialog = GetCurrentPanel();
    EXPECT_EQ("DialogPanel", dialog->GetTypeName());
    EXPECT_TRUE(dialog->IsShown());
    EXPECT_EQ("Load", agent.last_op);
}

TEST_F(SessionPanelTest, NewFromCancel) {
    // If there was a previous session but no changes were made and the current
    // session is not named, canceling should start a new session.
    auto &agent = *session_agent;

    panel->SetIsShown(true);

    agent.can_save  = false;
    agent.prev_name = "Prev.ems";

    // Cancel using the Escape key.
    Event event;
    event.device = Event::Device::kKeyboard;
    event.flags.Set(Event::Flag::kKeyPress);
    event.key_name = "Escape";
    EXPECT_TRUE(panel->HandleEvent(event));
    EXPECT_FALSE(panel->IsShown());

    // The New operation should have closed the Panel as "Done".
    EXPECT_EQ("Done", GetCloseResult());
    EXPECT_EQ("New", agent.last_op);
}

TEST_F(SessionPanelTest, NewAfterChanges) {
    auto &agent = *session_agent;

    agent.can_save = true;

    panel->SetIsShown(true);

    // If there are changes, ask before starting a new session. If the answer
    // is "Yes", close the SessionPanel as Done.
    ClickButtonPane("New");
    EXPECT_FALSE(panel->IsShown());
    auto dialog = GetCurrentPanel();
    EXPECT_EQ("DialogPanel", dialog->GetTypeName());
    EXPECT_TRUE(dialog->IsShown());
    ClickButtonPane("Button1");  // "Yes"
    EXPECT_FALSE(dialog->IsShown());
    EXPECT_FALSE(panel->IsShown());
    EXPECT_EQ("New", agent.last_op);
    EXPECT_EQ("Yes", GetCloseResult());
}

TEST_F(SessionPanelTest, Save) {
    auto &agent = *session_agent;

    agent.can_save = true;
    agent.cur_name = "Cur.ems";

    panel->SetIsShown(true);

    ClickButtonPane("Save");
    EXPECT_FALSE(panel->IsShown());
    EXPECT_EQ("Save", agent.last_op);
}

TEST_F(SessionPanelTest, SaveAs) {
    auto &agent = *session_agent;
    panel->SetIsShown(true);

    // Clicking the SaveAs button should show a FilePanel.
    ClickButtonPane("SaveAs");
    EXPECT_FALSE(panel->IsShown());
    auto file_panel = GetCurrentPanel();
    EXPECT_EQ("FilePanel", file_panel->GetTypeName());
    EXPECT_TRUE(file_panel->IsShown());
    // This has to be a new file for the Accept button to be enabled.
    SetTextInput("Input", GetDataPath("NewFile.ems").ToString());
    EXPECT_TRUE(IsButtonPaneEnabled("Accept"));
    ClickButtonPane("Accept");
    EXPECT_FALSE(file_panel->IsShown());
    EXPECT_FALSE(panel->IsShown());
    EXPECT_EQ("Save", agent.last_op);
}

TEST_F(SessionPanelTest, Export) {
    auto &agent = *session_agent;

    agent.export_name = "SomeModel";

    panel->SetIsShown(true);

    // Clicking the Export button should show a FilePanel.
    ClickButtonPane("Export");
    EXPECT_FALSE(panel->IsShown());
    auto file_panel = GetCurrentPanel();
    EXPECT_EQ("FilePanel", file_panel->GetTypeName());
    EXPECT_TRUE(file_panel->IsShown());
    // This has to be a new file for the Accept button to be enabled.
    SetTextInput("Input", GetDataPath("NewExportFile.stl").ToString());
    EXPECT_TRUE(IsButtonPaneEnabled("Accept"));
    ClickButtonPane("Accept");
    EXPECT_FALSE(file_panel->IsShown());
    EXPECT_FALSE(panel->IsShown());
    EXPECT_EQ("Export", agent.last_op);
}

TEST_F(SessionPanelTest, ExportFail) {
    auto &agent = *session_agent;

    agent.export_name = "SomeModel";

    panel->SetIsShown(true);

    // Clicking the Export button should show a FilePanel.
    agent.export_ok = false;
    ClickButtonPane("Export");
    EXPECT_FALSE(panel->IsShown());
    auto file_panel = GetCurrentPanel();
    EXPECT_EQ("FilePanel", file_panel->GetTypeName());
    EXPECT_TRUE(file_panel->IsShown());
    // This has to be a new file for the Accept button to be enabled.
    SetTextInput("Input", GetDataPath("NewExportFile.stl").ToString());
    EXPECT_TRUE(IsButtonPaneEnabled("Accept"));
    ClickButtonPane("Accept");
    EXPECT_FALSE(file_panel->IsShown());
    EXPECT_EQ("Export", agent.last_op);

    // An unsuccessful export should show a DialogPanel with the error message.
    auto dialog = GetCurrentPanel();
    EXPECT_EQ("DialogPanel", dialog->GetTypeName());
    EXPECT_TRUE(dialog->IsShown());
    ClickButtonPane("Button0");
    EXPECT_EQ("OK", GetCloseResult());
    EXPECT_FALSE(dialog->IsShown());
    EXPECT_FALSE(panel->IsShown());
}
