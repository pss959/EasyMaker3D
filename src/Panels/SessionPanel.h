#pragma once

#include <string>

#include "Panels/MainPanel.h"
#include "Util/FilePath.h"

namespace Parser { class Registry; }

/// SessionPanel is a derived MainPanel class that implements session
/// management.
class SessionPanel : public MainPanel {
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

    void OpenHelp_();
    void OpenSettings_();
    void ContinueSession_();
    void LoadSession_();
    void StartNewSession_();
    void SaveSession_(bool use_current_file);
    void ExportSelection_();

    FilePath GetInitialExportPath_();
    void LoadSessionFromPath_(const FilePath &path);
    void ReallyLoadSessionFromPath_(const FilePath &path);
    void SaveSessionToPath_(const FilePath &path);
    void ExportToPath_(const FilePath &path);
    void SetLastSessionPath_(const FilePath &path);

    friend class Parser::Registry;
};

typedef std::shared_ptr<SessionPanel> SessionPanelPtr;
