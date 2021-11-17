#pragma once

#include <string>

#include "Panels/Panel.h"

namespace Parser { class Registry; }

/// SessionPanel is a derived Panel class that implements session management.
class SessionPanel : public Panel {
  protected:
    SessionPanel() {}

    virtual void InitInterface() override;
    virtual void UpdateInterface() override;

  private:
    friend class Parser::Registry;

    void OpenHelp_();
    void OpenSettings_();
    void ContinueSession_();
    void LoadSession_();
    void StartNewSession_();
    void SaveSession_(bool use_current_file);
    void ExportSelection_();
};

typedef std::shared_ptr<SessionPanel> SessionPanelPtr;
