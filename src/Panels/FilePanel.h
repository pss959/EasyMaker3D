#pragma once

#include <string>

#include "Panels/Panel.h"

namespace Parser { class Registry; }

/// FilePanel is a derived Panel class that allows the user to select a file or
/// directory.
class FilePanel : public Panel {
  protected:
    FilePanel() {}

    virtual void InitInterface() override;
    virtual void UpdateInterface() override;

  private:
    friend class Parser::Registry;
};

typedef std::shared_ptr<FilePanel> FilePanelPtr;
