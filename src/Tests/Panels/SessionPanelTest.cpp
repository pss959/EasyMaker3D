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
    EXPECT_TRUE(panel->IsMovable());
    EXPECT_FALSE(panel->IsResizable());
    EXPECT_FALSE(panel->CanGripHover());
    EXPECT_NULL(panel->GetGripWidget(Point2f(0, 0)));
}

TEST_F(SessionPanelTest, Show) {
    EXPECT_ENUM_EQ(Panel::Status::kUnattached, panel->GetStatus());
    panel->SetStatus(Panel::Status::kVisible);
    EXPECT_ENUM_EQ(Panel::Status::kVisible, panel->GetStatus());
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
    panel->SetStatus(Panel::Status::kVisible);
    EXPECT_EQ("(No previous session)",
              cont->FindTypedSubPane<TextPane>("ButtonText")->GetText());
    EXPECT_FALSE(IsButtonPaneEnabled("Continue"));
    EXPECT_TRUE(IsButtonPaneEnabled("Load"));
    EXPECT_FALSE(IsButtonPaneEnabled("Save"));
    EXPECT_FALSE(IsButtonPaneEnabled("Export"));
    EXPECT_EQ(FindPane("New"), panel->GetFocusedPane());
    panel->SetStatus(Panel::Status::kUnattached);

    // Have previous but no current session name, no changes.
    agent.can_save    = false;
    agent.prev_name   = "Prev.ems";
    agent.cur_name    = "";
    agent.export_name = "";
    panel->SetStatus(Panel::Status::kVisible);
    EXPECT_EQ("Continue previous session [Prev.ems]",
              cont->FindTypedSubPane<TextPane>("ButtonText")->GetText());
    EXPECT_TRUE(IsButtonPaneEnabled("Continue"));
    EXPECT_TRUE(IsButtonPaneEnabled("Load"));
    EXPECT_FALSE(IsButtonPaneEnabled("Save"));
    EXPECT_FALSE(IsButtonPaneEnabled("Export"));
    // Focus should not have changed.
    EXPECT_EQ(FindPane("New"), panel->GetFocusedPane());
    panel->SetStatus(Panel::Status::kUnattached);

    // Have current but no previous session name, scene has changes and there
    // is a Model to export.
    agent.can_save    = true;
    agent.prev_name   = "";
    agent.cur_name    = "Cur.ems";
    agent.export_name = "SomeModel";
    panel->SetStatus(Panel::Status::kVisible);
    EXPECT_EQ("Continue current session [Cur.ems]",
              cont->FindTypedSubPane<TextPane>("ButtonText")->GetText());
    EXPECT_TRUE(IsButtonPaneEnabled("Continue"));
    EXPECT_TRUE(IsButtonPaneEnabled("Load"));
    EXPECT_TRUE(IsButtonPaneEnabled("Save"));
    EXPECT_TRUE(IsButtonPaneEnabled("Export"));
    // Focus should not have changed.
    EXPECT_EQ(FindPane("New"), panel->GetFocusedPane());
    panel->SetStatus(Panel::Status::kUnattached);

    // Have no current or previous session names, scene has changes.
    agent.can_save    = true;
    agent.prev_name   = "";
    agent.cur_name    = "";
    agent.export_name = "";
    panel->SetStatus(Panel::Status::kVisible);
    EXPECT_EQ("Continue current session",
              cont->FindTypedSubPane<TextPane>("ButtonText")->GetText());
    EXPECT_TRUE(IsButtonPaneEnabled("Continue"));
    EXPECT_TRUE(IsButtonPaneEnabled("Load"));
    EXPECT_FALSE(IsButtonPaneEnabled("Save"));
    EXPECT_FALSE(IsButtonPaneEnabled("Export"));
    // Focus should not have changed.
    EXPECT_EQ(FindPane("New"), panel->GetFocusedPane());
    panel->SetStatus(Panel::Status::kUnattached);
}

TEST_F(SessionPanelTest, ButtonFocusContinue) {
    // Make sure focus is on "Continue" button if it is enabled at
    // start. (Focus is persistent, so have to test this with a new instance.
    auto &agent = *session_agent;
    auto cont = FindTypedPane<ButtonPane>("Continue");

    agent.can_save  = false;
    agent.prev_name = "Prev.ems";
    agent.cur_name  = "";
    panel->SetStatus(Panel::Status::kVisible);
    EXPECT_EQ("Continue previous session [Prev.ems]",
              cont->FindTypedSubPane<TextPane>("ButtonText")->GetText());
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
    EXPECT_ENUM_EQ(Panel::Status::kHidden, panel->GetStatus());
    auto help_panel = GetCurrentPanel();
    EXPECT_EQ("HelpPanel", help_panel->GetTypeName());
    EXPECT_ENUM_EQ(Panel::Status::kVisible, help_panel->GetStatus());
    ClickButtonPane("Done");
    EXPECT_EQ("Done", GetCloseResult());
    EXPECT_ENUM_EQ(Panel::Status::kUnattached, help_panel->GetStatus());
    EXPECT_ENUM_EQ(Panel::Status::kVisible, panel->GetStatus());

    // Repeat with the "Settings" button.
    ClickButtonPane("Settings");
    EXPECT_ENUM_EQ(Panel::Status::kHidden, panel->GetStatus());
    auto settings_panel = GetCurrentPanel();
    EXPECT_EQ("SettingsPanel", settings_panel->GetTypeName());
    EXPECT_ENUM_EQ(Panel::Status::kVisible, settings_panel->GetStatus());
    ClickButtonPane("Cancel");
    EXPECT_EQ("Cancel", GetCloseResult());
    EXPECT_ENUM_EQ(Panel::Status::kUnattached, settings_panel->GetStatus());
    EXPECT_ENUM_EQ(Panel::Status::kVisible, panel->GetStatus());
}

TEST_F(SessionPanelTest, Continue) {
    // Session just started (no current session name, no changes) and the user
    // continued a previous session => load the previous session file.
    auto &agent = *session_agent;

    agent.can_save  = false;
    agent.prev_name = "Prev.ems";
    agent.cur_name  = "";
    panel->SetStatus(Panel::Status::kVisible);

    // An unsuccessful load should show a DialogPanel with the error message.
    agent.load_ok = false;
    ClickButtonPane("Continue");
    EXPECT_EQ("Load", agent.last_op);
    EXPECT_ENUM_EQ(Panel::Status::kHidden, panel->GetStatus());
    auto dialog = GetCurrentPanel();
    EXPECT_EQ("DialogPanel", dialog->GetTypeName());
    EXPECT_ENUM_EQ(Panel::Status::kVisible, dialog->GetStatus());
    ClickButtonPane("Button0");
    EXPECT_EQ("OK", GetCloseResult());
    EXPECT_ENUM_EQ(Panel::Status::kUnattached, dialog->GetStatus());
    EXPECT_ENUM_EQ(Panel::Status::kVisible, panel->GetStatus());

    // A successful load should close the SessionPanel.
    agent.load_ok = true;
    ClickButtonPane("Continue");
    EXPECT_EQ("Load", agent.last_op);
    EXPECT_ENUM_EQ(Panel::Status::kUnattached, panel->GetStatus());
    EXPECT_EQ("Done", GetCloseResult());
}

TEST_F(SessionPanelTest, Load) {
    auto &agent = *session_agent;

    panel->SetStatus(Panel::Status::kVisible);

    // Force the "really load" Dialog to appear.
    agent.can_save = true;
    agent.save_ok  = true;

    // Clicking the Load button should show a FilePanel.
    ClickButtonPane("Load");
    EXPECT_ENUM_EQ(Panel::Status::kHidden, panel->GetStatus());
    auto file_panel = GetCurrentPanel();
    EXPECT_EQ("FilePanel", file_panel->GetTypeName());
    EXPECT_ENUM_EQ(Panel::Status::kVisible, file_panel->GetStatus());

    // This has to be a valid file for the Accept button to be enabled.
    SetTextInput("Input", GetDataPath("Sessions/Empty.ems").ToString());
    EXPECT_TRUE(IsButtonPaneEnabled("Accept"));

    // Accepting the file to load should close the FilePanel and open the
    // DialogPanel.
    ClickButtonPane("Accept");
    EXPECT_EQ("Accept", GetCloseResult());
    EXPECT_ENUM_EQ(Panel::Status::kUnattached, file_panel->GetStatus());
    auto dialog = GetCurrentPanel();
    EXPECT_EQ("DialogPanel", dialog->GetTypeName());
    EXPECT_ENUM_EQ(Panel::Status::kVisible, dialog->GetStatus());

    // Click the "Yes" button to really load the session and close the
    // SessionPanel.
    ClickButtonPane("Button1");  // "Yes".
    EXPECT_EQ("Yes", GetCloseResult());
    EXPECT_ENUM_EQ(Panel::Status::kUnattached, dialog->GetStatus());
    EXPECT_ENUM_EQ(Panel::Status::kUnattached, panel->GetStatus());
    EXPECT_EQ("Load", agent.last_op);
}

TEST_F(SessionPanelTest, LoadFail) {
    auto &agent = *session_agent;

    panel->SetStatus(Panel::Status::kVisible);

    // Clicking the Load button should show a FilePanel.
    agent.load_ok = false;
    ClickButtonPane("Load");
    EXPECT_ENUM_EQ(Panel::Status::kHidden, panel->GetStatus());
    auto file_panel = GetCurrentPanel();
    EXPECT_EQ("FilePanel", file_panel->GetTypeName());
    EXPECT_ENUM_EQ(Panel::Status::kVisible, file_panel->GetStatus());

    // This has to be a valid file for the Accept button to be enabled.
    SetTextInput("Input", GetDataPath("Sessions/Empty.ems").ToString());
    EXPECT_TRUE(IsButtonPaneEnabled("Accept"));

    // Accepting the file to load should show the error DialogPanel.
    ClickButtonPane("Accept");
    EXPECT_EQ("Accept", GetCloseResult());
    EXPECT_ENUM_EQ(Panel::Status::kUnattached, file_panel->GetStatus());
    auto dialog = GetCurrentPanel();
    EXPECT_EQ("DialogPanel", dialog->GetTypeName());
    EXPECT_ENUM_EQ(Panel::Status::kVisible, dialog->GetStatus());
    EXPECT_EQ("Load", agent.last_op);
}

TEST_F(SessionPanelTest, NewFromCancel) {
    // If there was a previous session but no changes were made and the current
    // session is not named, canceling should start a new session.
    auto &agent = *session_agent;

    panel->SetStatus(Panel::Status::kVisible);

    agent.can_save  = false;
    agent.prev_name = "Prev.ems";

    // Cancel using the Escape key.
    Event event;
    event.device = Event::Device::kKeyboard;
    event.flags.Set(Event::Flag::kKeyPress);
    event.key_name = "Escape";
    EXPECT_TRUE(panel->HandleEvent(event));
    EXPECT_ENUM_EQ(Panel::Status::kUnattached, panel->GetStatus());

    // The New operation should have closed the Panel as "Done".
    EXPECT_EQ("Done", GetCloseResult());
    EXPECT_EQ("New", agent.last_op);
}

TEST_F(SessionPanelTest, NewAfterChanges) {
    auto &agent = *session_agent;

    agent.can_save = true;

    panel->SetStatus(Panel::Status::kVisible);

    // If there are changes, ask before starting a new session. If the answer
    // is "Yes", close the SessionPanel as Done.
    ClickButtonPane("New");
    EXPECT_ENUM_EQ(Panel::Status::kHidden, panel->GetStatus());
    auto dialog = GetCurrentPanel();
    EXPECT_EQ("DialogPanel", dialog->GetTypeName());
    EXPECT_ENUM_EQ(Panel::Status::kVisible, dialog->GetStatus());
    ClickButtonPane("Button1");  // "Yes"
    EXPECT_ENUM_EQ(Panel::Status::kUnattached, dialog->GetStatus());
    EXPECT_ENUM_EQ(Panel::Status::kUnattached, panel->GetStatus());
    EXPECT_EQ("New", agent.last_op);
    EXPECT_EQ("Yes", GetCloseResult());
}

TEST_F(SessionPanelTest, Save) {
    auto &agent = *session_agent;

    agent.can_save = true;
    agent.cur_name = "Cur.ems";

    panel->SetStatus(Panel::Status::kVisible);

    ClickButtonPane("Save");
    EXPECT_ENUM_EQ(Panel::Status::kUnattached, panel->GetStatus());
    EXPECT_EQ("Save", agent.last_op);
}

TEST_F(SessionPanelTest, SaveAs) {
    auto &agent = *session_agent;
    panel->SetStatus(Panel::Status::kVisible);

    // Clicking the SaveAs button should show a FilePanel.
    ClickButtonPane("SaveAs");
    EXPECT_ENUM_EQ(Panel::Status::kHidden, panel->GetStatus());
    auto file_panel = GetCurrentPanel();
    EXPECT_EQ("FilePanel", file_panel->GetTypeName());
    EXPECT_ENUM_EQ(Panel::Status::kVisible, file_panel->GetStatus());
    // This has to be a new file for the Accept button to be enabled.
    SetTextInput("Input", GetDataPath("NewFile.ems").ToString());
    EXPECT_TRUE(IsButtonPaneEnabled("Accept"));
    ClickButtonPane("Accept");
    EXPECT_ENUM_EQ(Panel::Status::kUnattached, file_panel->GetStatus());
    EXPECT_ENUM_EQ(Panel::Status::kUnattached, panel->GetStatus());
    EXPECT_EQ("Save", agent.last_op);
}

TEST_F(SessionPanelTest, Export) {
    auto &agent = *session_agent;

    agent.export_name = "SomeModel";

    panel->SetStatus(Panel::Status::kVisible);

    // Clicking the Export button should show a FilePanel.
    ClickButtonPane("Export");
    EXPECT_ENUM_EQ(Panel::Status::kHidden, panel->GetStatus());
    auto file_panel = GetCurrentPanel();
    EXPECT_EQ("FilePanel", file_panel->GetTypeName());
    EXPECT_ENUM_EQ(Panel::Status::kVisible, file_panel->GetStatus());
    // This has to be a new file for the Accept button to be enabled.
    SetTextInput("Input", GetDataPath("NewExportFile.stl").ToString());
    EXPECT_TRUE(IsButtonPaneEnabled("Accept"));
    ClickButtonPane("Accept");
    EXPECT_ENUM_EQ(Panel::Status::kUnattached, file_panel->GetStatus());
    EXPECT_ENUM_EQ(Panel::Status::kUnattached, panel->GetStatus());
    EXPECT_EQ("Export", agent.last_op);
}

TEST_F(SessionPanelTest, ExportFail) {
    auto &agent = *session_agent;

    agent.export_name = "SomeModel";

    panel->SetStatus(Panel::Status::kVisible);

    // Clicking the Export button should show a FilePanel.
    agent.export_ok = false;
    ClickButtonPane("Export");
    EXPECT_ENUM_EQ(Panel::Status::kHidden, panel->GetStatus());
    auto file_panel = GetCurrentPanel();
    EXPECT_EQ("FilePanel", file_panel->GetTypeName());
    EXPECT_ENUM_EQ(Panel::Status::kVisible, file_panel->GetStatus());
    // This has to be a new file for the Accept button to be enabled.
    SetTextInput("Input", GetDataPath("NewExportFile.stl").ToString());
    EXPECT_TRUE(IsButtonPaneEnabled("Accept"));
    ClickButtonPane("Accept");
    EXPECT_ENUM_EQ(Panel::Status::kUnattached, file_panel->GetStatus());
    EXPECT_EQ("Export", agent.last_op);

    // An unsuccessful export should show a DialogPanel with the error message.
    auto dialog = GetCurrentPanel();
    EXPECT_EQ("DialogPanel", dialog->GetTypeName());
    EXPECT_ENUM_EQ(Panel::Status::kVisible, dialog->GetStatus());
    ClickButtonPane("Button0");
    EXPECT_EQ("OK", GetCloseResult());
    EXPECT_ENUM_EQ(Panel::Status::kUnattached, dialog->GetStatus());
    EXPECT_ENUM_EQ(Panel::Status::kUnattached, panel->GetStatus());
}
