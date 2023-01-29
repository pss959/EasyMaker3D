#pragma once

#include <string>

#include "Base/Memory.h"
#include "Panels/Panel.h"

DECL_SHARED_PTR(HelpPanel);

namespace Parser { class Registry; }

/// HelpPanel is a derived Panel class that implements help management.
///
/// \ingroup Panels
class HelpPanel : public Panel {
  protected:
    HelpPanel() {}

    virtual void InitInterface() override;
    virtual void UpdateInterface() override;

  private:
    void OpenPage_(const std::string &page_name);
    void OpenIssue_();

    friend class Parser::Registry;
};
