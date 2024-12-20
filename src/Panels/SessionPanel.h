//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include "Enums/FileFormat.h"
#include "Panels/Panel.h"
#include "Util/FilePath.h"
#include "Util/Memory.h"

DECL_SHARED_PTR(SessionPanel);

namespace Parser { class Registry; }

/// SessionPanel is a derived Panel class that implements session management.
///
/// \ingroup Panels
class SessionPanel : public Panel {
  protected:
    SessionPanel() {}

    virtual void InitInterface() override;
    virtual void UpdateInterface() override;

    /// Redefines this interpret "Cancel" to either start a new session or
    /// continue the current session.
    virtual void Close(const Str &result) override;

  private:
    /// Reasons to open a FilePanel to choose a file.
    enum class FileTarget_ {
        kLoadSession,
        kSaveSession,
        kExport,
    };

    /// Stores the session file extension for convenience.
    Str extension_;

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
    void ExportToPath_(const FilePath &path, FileFormat format);
    void SetLastSessionPath_(const FilePath &path);

    friend class Parser::Registry;
};
