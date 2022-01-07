#pragma once

#include <string>

#include "Panels/Panel.h"
#include "Util/FilePath.h"

namespace Parser { class Registry; }

/// SessionPanel is a derived Panel class that implements session management.
class SessionPanel : public Panel {
  public:
    /// Redefines this to set up the FilePanel.
    virtual void InitReplacementPanel(Panel &new_panel) override;

  protected:
    SessionPanel() {}

    virtual void InitInterface() override;
    virtual void UpdateInterface() override;

  private:
    /// Reasons to open a FilePanel to choose a file.
    enum class FileTarget_ {
        kLoadSession,
        kSaveSession,
        kExport,
    };

    /// Saves the reason the FilePanel is opened.
    FileTarget_ file_panel_target_;

    void OpenHelp_();
    void OpenSettings_();
    void ContinueSession_();
    void LoadSession_();
    void StartNewSession_();
    void SaveSession_(bool use_current_file);
    void ExportSelection_();

    void SaveSessionToPath_(const Util::FilePath &path);
    void SetLastSessionPath_(const Util::FilePath &path);
    void ChooseFile_(FileTarget_ target);
    static std::string GetFilePanelTitle_(FileTarget_ target);
    Util::FilePath GetInitialPath_(FileTarget_ target);

    friend class Parser::Registry;
};

typedef std::shared_ptr<SessionPanel> SessionPanelPtr;
